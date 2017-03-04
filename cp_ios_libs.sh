#!/usr/bin/env bash

src_dir=./.build_ios
dest_dir=../UrhoDemo/VisioninDemo/VisioninDemo/urho3d/

cp ${src_dir}/lib/libUrho3D.a ${dest_dir}
cp ${src_dir}/Source/ThirdParty/FreeType/Debug-iphoneos/libFreeType.a ${dest_dir}
cp ${src_dir}/Source/ThirdParty/JO/Debug-iphoneos/libJO.a ${dest_dir}
cp ${src_dir}/Source/ThirdParty/LZ4/Debug-iphoneos/libLZ4.a ${dest_dir}
cp ${src_dir}/Source/ThirdParty/PugiXml/Debug-iphoneos/libPugiXml.a ${dest_dir}
cp ${src_dir}/Source/ThirdParty/SDL/Debug-iphoneos/libSDL.a ${dest_dir}
cp ${src_dir}/Source/ThirdParty/StanHull/Debug-iphoneos/libStanHull.a ${dest_dir}

sync
