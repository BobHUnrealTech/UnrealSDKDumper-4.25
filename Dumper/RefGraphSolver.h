#pragma once
#include <unordered_map>
#include <vector>
#include <string>
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

  // map the TypeName -> PackageName
  static std::unordered_map<std::string, std::string> typeDefMap;

  static void LoadUnrealPackageDef() {
    // ������������Դ���һЩ��Ķ��壬��Щ���岻�Ǵ���Ϸ��dump�ģ��Ǵ���������пٳ����ģ���Ҫ��������
  }

  static void LoadPackageDef(UE_UPackage& package) {
    LoadUnrealPackageDef();
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
    if (verboseDebug) {
      printf("Loaded all packages defs!");
    }
  }

  static void BuildRefGraph(UE_UPackage& package) {

  }
public:
  static void Process(std::vector<UE_UPackage>& packages) {
    for (UE_UPackage& package : packages) {
      LoadPackageDef(package);
    }
    for (UE_UPackage& package : packages) {
      BuildRefGraph(package);
    }
  }

};

#endif