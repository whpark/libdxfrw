// cad2json.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <filesystem>

#include <iostream>
#include <fstream>

#include "converter.h"

bool convertFile(std::filesystem::path in, std::filesystem::path out){
    std::filesystem::path p;

    bool badState = false;
    //verify if input file exist
    std::ifstream ifs;
    ifs.open(in, std::ifstream::in);
    badState = ifs.fail();
    ifs.close();
    if (badState) {
        //std::cout << "Error can't open " << inName << std::endl;
        return false;
    }

    //gtl::shape::CShapeOStreamText ar(ofs);

    //All ok proceed whit conversion
    //class to store file read:
    //First read a dwg or dxf file
    auto loader = std::make_unique<gtl::shape::CDXFConverter>();

    //boost::archive::text_oarchive ar;

    badState = loader->fileImport(in);
    if (!badState) {
        //std::cout << "Error reading file " << inName << std::endl;
        return false;
    }

    std::ofstream ofs(out, std::ios_base::binary);
    gtl::shape::oarchive_json ar(ofs);
    loader->Export(ar);

    return badState;
}

int wmain(int argc, wchar_t *argv[]) {
    if (argc < 3) {
        return 1;
    }

    //parse params.
    std::wstring fileName = argv[1];
    std::wstring outName = argv[2];

    bool ok = convertFile(fileName, outName);
    if (ok)
        return 0;
    else
        return 1;
}

