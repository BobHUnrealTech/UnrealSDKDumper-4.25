#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <set>
#include <cassert>
#include <fmt/core.h>
#include <fstream>
#include "wrappers.h"

/*
* Author: BobH
* Time: 2023.7.18
* Usage: ��������ϵ����ͼ, ��������, �����ȷͷ�ļ�˳��
*/

const bool verboseDebug = 1;

#ifndef REFGRAPHSOLVER
#define REFGRAPHSOLVER

class RefGraphSolver
{
  struct Node {
    Node() {
      packageName = "";
      indeg = 0;
      outdeg = 0;
      neighbors = std::vector<Node*>();
    }
    Node(std::string name) {
      static int id = 0;
      debug_id = ++id;
      packageName = name;
      indeg = 0;
      outdeg = 0;
      neighbors = std::vector<Node*>();
    }
    std::string packageName;
    std::vector<Node*> neighbors;
    int indeg, outdeg;
    int debug_id;
  };

  // map the TypeName -> PackageName
  static std::unordered_map<std::string, std::string> typeDefMap;

  // map the packageName -> NodePointer
  static std::unordered_map<std::string, Node*> nodesMap;

  // Store all the package nodes for topo sort
  static std::vector<Node*> packageNodes;

  // Ӳ���Դ������Ͷ��壬��Ҫ�ֶ���ȡд��ȥ��
  static std::vector<std::string> BasicTypes;
  static std::vector<std::string> CoreUObject;
  static std::vector<std::string> CppTypes;

  static void PushNode(std::string packageName) {
    auto newNode = new Node(packageName);
    nodesMap[packageName] = newNode;
    packageNodes.push_back(newNode);
  }

  static void LoadUnrealPackageDef() {
    // ������������Դ���һЩ��Ķ��壬��Щ���岻�Ǵ���Ϸ��dump�ģ��Ǵ���������пٳ����ģ���Ҫ��������
    for (auto ClassName : BasicTypes) {
      typeDefMap[ClassName] = "BasicTypes";
    }
    for (auto ClassName : CoreUObject) {
      typeDefMap[ClassName] = "CoreUObject";
    }
    for (auto ClassName : CppTypes) {
      typeDefMap[ClassName] = "CppTypes";
    }
    PushNode("BasicTypes");
    PushNode("CoreUObject");
    PushNode("CppTypes");
  }



  static std::string GetPureTypeName(std::string fullTypeName) {
    auto removeSubstring = [](std::string& input, const std::string toRemove) {
      size_t pos = input.find(toRemove);
      while (pos != std::string::npos) {
        input.erase(pos, toRemove.length());
        pos = input.find(toRemove, pos);
      }
    };
    auto trim = [](std::string& input) {
      size_t start = 0;
      size_t end = input.length() - 1;

      // Find the first non-space character from the beginning of the string
      while (start <= end && std::isspace(input[start])) {
        start++;
      }

      // Find the last non-space character from the end of the string
      while (end >= start && std::isspace(input[end])) {
        end--;
      }

      // Return the trimmed substring
      input = input.substr(start, end - start + 1);
    };
    std::string res = fullTypeName;
    removeSubstring(res, "enum");
    removeSubstring(res, "class");
    removeSubstring(res, "struct");
    removeSubstring(res, "*");
    trim(res);
    return res;
  }

  static std::vector<std::string> GetGenericTypes(std::string genericTypeName) {
    auto removeSubstring = [](std::string& input, const std::string toRemove) {
      size_t pos = input.find(toRemove);
      while (pos != std::string::npos) {
        input.erase(pos, toRemove.length());
        pos = input.find(toRemove, pos);
      }
    };
    auto splitString = [](const std::string& input, const std::string& delimiters) {
      std::vector<std::string> tokens;
      std::string::size_type startPos = 0;
      std::string::size_type endPos = 0;

      while (endPos != std::string::npos) {
        endPos = input.find_first_of(delimiters, startPos);

        // ����ҵ��˷ָ���
        if (endPos != startPos) {
          std::string token = input.substr(startPos, endPos - startPos);
          tokens.push_back(token);
        }

        // �����ָ���������������һ���Ӵ�
        if (endPos != std::string::npos) {
          startPos = endPos + 1;
        }
      }
      tokens.pop_back();
      return tokens;
    };
    removeSubstring(genericTypeName, " ");
    const std::string delimiters = "<>,";
    return splitString(genericTypeName, delimiters);
  }

  static void LoadPackageDef(UE_UPackage& package) {
    std::string packageName = package.GetObject().GetName();
    for (auto& klass : package.Classes) {
      typeDefMap[klass.ClassName] = packageName;
    }
    for (auto& stru : package.Structures) {
      typeDefMap[stru.ClassName] = packageName;
    }
    for (auto& enums : package.Enums) {
      typeDefMap[enums.EnumName] = packageName;
    }

    PushNode(packageName);

  }

  static bool AddEdge(std::string packageReferer, std::string packageReferee) {
    static std::ofstream output("relation.txt");
    // packageReferer -> packageReferee
    if (nodesMap.count(packageReferer) == 0) {
      printf("Could not found Referer class node \"%s\"!", packageReferer.c_str());
      return false;
    }
    if (nodesMap.count(packageReferee) == 0) {
      printf("Could not found Referee class node \"%s\"!", packageReferee.c_str() );
      return false;
    }
    auto pReferer = nodesMap[packageReferer];
    auto pReferee = nodesMap[packageReferee];
    pReferer->outdeg++;
    pReferee->indeg++;
    pReferer->neighbors.push_back(pReferee);
    output << pReferer->packageName << " -> " << pReferee->packageName << std::endl;
    return true;
  }

  static void FixUndefinedClassMember(UE_UPackage::Member& member) {
    // �޸��ṹ��һЩû���ֵĽṹ��ֱ����char����
    member.Type = "char";
    member.Name += fmt::format("[{:#0x}]", member.Size);
  }

  static void BuildRefGraph(UE_UPackage& package) {
    std::string packageName = package.GetObject().GetName();


    // �洢���������ͣ�ȥ��
    std::set<std::string> refTypes;

    // �洢������package��ȥ��
    std::set <std::string> refPackages;

    auto processStruct = [&refTypes, &refPackages](UE_UPackage::Struct& klass) {
      // ����̳�����
      auto& superName = klass.SuperName;
      if (superName == "FNone" && verboseDebug) {
        // printf("EmptySuperClass: %s\n", klass.ClassName.c_str());
      }
      assert(superName.find("<") == std::string::npos);  // Should not be possible?
      if (superName != "FNone" && superName != "") refTypes.insert(superName);

      // �����Ա��������
      for (auto& member : klass.Members) {
        auto purename = GetPureTypeName(member.Type);
        if (purename.find("<") != std::string::npos) {
          // generic type
          auto genericTypes = GetGenericTypes(purename);
          bool should_fix = false;
          for (auto& tname : genericTypes) {
            assert(tname != "");
            if (typeDefMap.count(tname) == 0) {
              should_fix = true;
              FixUndefinedClassMember(member);
              break;
            }
          }
          if(!should_fix)
            for (auto& tname : genericTypes) {
              assert(tname != "");
              refTypes.insert(tname);
            }
        }
        else {
          assert(purename != "");
          if (typeDefMap.count(purename) == 0)
            FixUndefinedClassMember(member);
          else
            refTypes.insert(purename);
        }
      }

      // ��������������
      for (auto& function : klass.Functions) {
        // ������ֵ
        bool deleteFunction = false; // ��������������ֵ�����⣬�Ǿ����ó�void��
        auto purename = GetPureTypeName(function.RetType);
        if (purename.find("<") != std::string::npos) {
          // generic type
          auto genericTypes = GetGenericTypes(purename);
          bool should_fix = false;
          for (auto& tname : genericTypes) {
            assert(tname != "");
            if (typeDefMap.count(tname) == 0) {
              should_fix = true;
              function.RetType = "void";
              function.CppName = "void " + function.FuncName;
              break;
            }
          }
          if(!should_fix)
            for (auto& tname : genericTypes) {
              assert(tname != "");
              refTypes.insert(tname);
            }
        }
        else {
          assert(purename != "");
          if (typeDefMap.count(purename) == 0) {
            function.RetType = "void";
            function.CppName = "void " + function.FuncName;
          }
          else {
            refTypes.insert(purename);
          }
        }
        // �������
        for (auto paramtype : function.ParamTypes) {
          auto purename = GetPureTypeName(paramtype);
          if (purename.find("<") != std::string::npos) {
            // generic type
            auto genericTypes = GetGenericTypes(purename);
            bool should_fix = false;
            for (auto& tname : genericTypes) {
              assert(tname != "");
              if (typeDefMap.count(tname) == 0) {
                should_fix = true;
                function.Params = "/*" + function.Params + "*/";
                function.ParamTypes.clear();
                goto end;
              }
            }
            if(!should_fix)
              for (auto& tname : genericTypes) {
                assert(tname != "");
                refTypes.insert(tname);
              }
          }
          else {
            assert(purename != "");
            if (typeDefMap.count(purename) == 0) {
              function.Params = "/*" + function.Params + "*/";
              function.ParamTypes.clear();
              goto end;
            }
            else {
              refTypes.insert(purename);
            }
          }
        }
        end:;
      }
    };

    // �ҳ�������������
    for (auto& klass : package.Classes) {
      processStruct(klass);
    }
    for (auto& klass : package.Structures) {
      processStruct(klass);
    }
    
    // �ҳ�����Package����
    for (auto& refType : refTypes) {
      if (typeDefMap.count(refType) == 0) {
        printf("[Warning] Cannot find type \"%s\" in any package!\n", refType.c_str());
        continue;
      }
      refPackages.insert(typeDefMap[refType]);
    }

    // ���ӹ�ϵ
    for(auto& targetPackage : refPackages) {
      AddEdge(packageName, targetPackage);
    }
  }
public:
  static void Process(std::vector<UE_UPackage>& packages) {
    LoadUnrealPackageDef();
    for (UE_UPackage& package : packages) {
      LoadPackageDef(package);
    }

    if (verboseDebug) {
      printf("\nLoaded all packages defs! \n");
    }

    for (UE_UPackage& package : packages) {
      BuildRefGraph(package);
    }
  }

};

#endif