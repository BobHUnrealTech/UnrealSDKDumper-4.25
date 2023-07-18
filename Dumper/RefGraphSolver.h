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

#ifndef REFGRAPHSOLVER
#define REFGRAPHSOLVER

class RefGraphSolver
{

  // map the TypeName -> PackageName
  static std::vector<std::string, std::string> typeDefMap;

  static void LoadPackageDef(UE_UPackage& package) {
    std::string packageName = package.GetObject().GetName();
    if (packageName == "Engine") {
      for (auto& classes : package.Classes) {
        printf("test: %s\n", classes.CppName.c_str());
      }
    }
  }

  static void BuildRefGraph() {

  }
public:
  static void Process(std::vector<UE_UPackage>& packages) {
    for (UE_UPackage& package : packages) {
      LoadPackageDef(package);
    }
  }

};

#endif