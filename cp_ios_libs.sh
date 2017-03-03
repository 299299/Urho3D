#!/usr/bin/env bash

cp ./.build_ios/lib/libUrho3D.a ../Test/proj/VisioninDemo/VisioninDemo/urho3d/
cp ./.build_ios/Source/ThirdParty/FreeType/Debug-iphoneos/libFreeType.a ../Test/proj/VisioninDemo/VisioninDemo/urho3d/
cp ./.build_ios/Source/ThirdParty/JO/Debug-iphoneos/libJO.a ../Test/proj/VisioninDemo/VisioninDemo/urho3d/
cp ./.build_ios/Source/ThirdParty/LZ4/Debug-iphoneos/libLZ4.a ../Test/proj/VisioninDemo/VisioninDemo/urho3d/
cp ./.build_ios/Source/ThirdParty/PugiXml/Debug-iphoneos/libPugiXml.a ../Test/proj/VisioninDemo/VisioninDemo/urho3d/
cp ./.build_ios/Source/ThirdParty/SDL/Debug-iphoneos/libSDL.a ../Test/proj/VisioninDemo/VisioninDemo/urho3d/
cp ./.build_ios/Source/ThirdParty/StanHull/Debug-iphoneos/libStanHull.a ../Test/proj/VisioninDemo/VisioninDemo/urho3d/

sync
