// cad2json.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>

#include "converter.h"

bool convertFile(std::string inName, std::string outName){
    bool badState = false;
    //verify if input file exist
    std::ifstream ifs;
    ifs.open (inName.c_str(), std::ifstream::in);
    badState = ifs.fail();
    ifs.close();
    if (badState) {
        //std::cout << "Error can't open " << inName << std::endl;
        return false;
    }

    std::ofstream ofs(outName, std::ios_base::binary);
    //gtl::shape::CShapeOStreamText ar(ofs);

    //All ok proceed whit conversion
    //class to store file read:
    //First read a dwg or dxf file
    auto loader = std::make_unique<gtl::shape::CDXFConverter>();

    //boost::archive::text_oarchive ar;

    badState = loader->fileImport( inName );
    if (!badState) {
        //std::cout << "Error reading file " << inName << std::endl;
        return false;
    }

    gtl::shape::oarchive_json ar(ofs);
    loader->Export(ar);

    //loader->fileExport(outName);

    //loader->fileExportShape(outName);

    ////And write a dxf file
    //dx_iface *output = new dx_iface();
    //badState = output->fileExport(outName, ver, binary, &fData);
    //delete input;
    //delete output;

    return badState;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }

    //parse params.
    std::string fileName = argv[1];
    std::string outName = argv[2];

    bool ok = convertFile(fileName, outName);
    if (ok)
        return 0;
    else
        return 1;
}

