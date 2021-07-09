#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "Loadmesh.h"



struct data attrib_to_data(tinyobj::ObjReader reader, std::string inputfile, tinyobj::ObjReaderConfig reader_config){
    if (!reader.ParseFromFile(inputfile, reader_config)) {
      if (!reader.Error().empty()) {
          std::cerr << "TinyObjReader: " << reader.Error();
      }
      exit(1);
    }

    if (!reader.Warning().empty()) {
      std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    unsigned long vertices_length = 0;
    unsigned long texture_length = 0;

    for (size_t s = 0; s < shapes.size(); s++) {
      size_t index_offset = 0;
      for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
        size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++) {

          tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
          vertices_length += 3;

          // if there is coordinates for the normal vector :

          /*if (idx.normal_index >= 0) {
              data_length += 3;

          }*/

          if (idx.texcoord_index >= 0) {
              texture_length += 2;
          }

         }
        index_offset += fv;
      }
    }

    std::vector<float> val(vertices_length + texture_length);

    int cursor_vertices = 0;
    int cursor_texture = 0;

    for (size_t s = 0; s < shapes.size(); s++) {
      // Loop over faces(polygon)
      size_t index_offset = 0;
      for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
        size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

        // Loop over vertices in the face.
        for (size_t v = 0; v < fv; v++) {
          // access to vertex
          tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
          val[cursor_vertices] = attrib.vertices[3*size_t(idx.vertex_index)+0];
          val[cursor_vertices + 1] = attrib.vertices[3*size_t(idx.vertex_index)+1];
          val[cursor_vertices + 2] = attrib.vertices[3*size_t(idx.vertex_index)+2];

          cursor_vertices += 3;

          // Check if `normal_index` is zero or positive. negative = no normal data
          /*if (idx.normal_index >= 0) {
            data[cursor] = attrib.normals[3*size_t(idx.normal_index)+0];
            data[cursor + 1] = attrib.normals[3*size_t(idx.normal_index)+1];
            data[cursor + 2] = attrib.normals[3*size_t(idx.normal_index)+2];

            cursor += 3;
          }*/

          // Check if `texcoord_index` is zero or positive. negative = no texcoord data
          //if (idx.texcoord_index >= 0) {
          val[vertices_length + cursor_texture] = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
          val[vertices_length + cursor_texture + 1] = attrib.texcoords[2*size_t(idx.texcoord_index)+1];

          cursor_texture += 2;
          //}

        }
        index_offset += fv;

      }
    }

    struct data d = {val, vertices_length, texture_length};

    return d;
}
