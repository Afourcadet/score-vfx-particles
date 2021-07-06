#!/bin/bash
rm -rf release
mkdir -p release

cp -rf particles *.{hpp,cpp,txt,json} LICENSE release/

mv release score-addon-particles
7z a score-addon-particles.zip score-addon-particles
