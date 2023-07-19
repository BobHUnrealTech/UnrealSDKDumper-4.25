#pragma once
#include <string>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include "resource.h"
/*
* Author: BobH
* Time: 2023.7.19
* Usage: �����Դ���ͷ�ļ��ĵ�����������Ҫ�ж���Ϸ�İ汾��
*/


#ifndef ENGINEHEADEREXPORT
#define ENGINEHEADEREXPORT

class EngineHeaderExport
{
  static bool LoadResourceText(std::string& str, int resourceID);

  static void ReplaceVAR(std::string& str);


public:
  
  static void Process(std::filesystem::path dir) {
    const static std::vector<std::pair<int, std::string>> EngineFiles = {
      std::make_pair(BASICTYPES_PACKAGE, "BasicTypes_Package.h"),
      std::make_pair(BASICTYPES_CLASSES, "BasicTypes_Classes.h"),
      std::make_pair(BASICTYPES_STRUCT, "BasicTypes_Structs.h"),
      std::make_pair(COREUOBJECT_PACKAGE, "CoreUObject_Package.h"),
      std::make_pair(COREUOBJECT_CLASSES, "CoreUObject_Classes.h"),
      std::make_pair(COREUOBJECT_STRUCT, "CoreUObject_Structs.h"),
    };
    // �������������ļ�
    for (auto& enginefile : EngineFiles) {
      std::ofstream output(dir / enginefile.second);
      std::string content;
      if (!LoadResourceText(content, enginefile.first)) {
        printf("[ERROR] Fail to read engine header file: %s\n", enginefile.second.c_str());
        continue;
      }
      ReplaceVAR(content);
      // fmt::print(file, "{}", content);
      output << content;
    }
  }
};

#endif
