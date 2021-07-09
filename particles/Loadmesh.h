#ifndef LOADMESH_H
#define LOADMESH_H
#include "tiny_obj_loader.h"
#include <iostream>
#include <QApplication>

struct data{
    std::vector<float> values;
    unsigned long vertices_length;
    unsigned long texture_length;
};

struct data attrib_to_data(tinyobj::ObjReader reader, std::string inputfile, tinyobj::ObjReaderConfig reader_config);

#endif // LOADMESH_H
