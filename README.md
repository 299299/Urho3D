
=======
![Urho3D logo](https://raw.githubusercontent.com/urho3d/Urho3D/master/bin/Data/Textures/LogoLarge.png)

#Urho3D

**Urho3D** is a free lightweight, cross-platform 2D and 3D game engine implemented in C++ and released under the MIT license. Greatly inspired by OGRE and Horde3D.

Main website: [http://urho3d.github.io/](http://urho3d.github.io/)

##License
Licensed under the MIT license, see [License.txt](https://github.com/urho3d/Urho3D/blob/master/License.txt) for details.

##Credits
Urho3D development, contributions and bugfixes by:
- Lasse Öörni (loorni@gmail.com, AgentC at GameDev.net)
- Wei Tjong Yao
- Aster Jian
- Vivienne Anthony
- Colin Barrett
- Erik Beran
- Danny Boisvert
- Carlo Carollo
- Pete Chown
- Christian Clavet
- Sebastian Delatorre (primitivewaste)
- Josh Engebretson
- Chris Friesen
- Alex Fuller
- Mika Heinonen
- Jukka Jylänki
- Graham King
- Jason Kinzer
- Gunnar Kriik
- Ali Kämäräinen
- Pete Leigh
- Thorbjørn Lindeijer
- Xavier Maupeu
- Jonne Nauha
- Paul Noome
- David Palacios
- Alex Parlett
- Jordan Patterson
- Anton Petrov
- Vladimir Pobedinsky
- Pranjal Raihan
- Nick Royer
- Miika Santala
- Hualin Song
- James Thomas
- Joshua Tippetts
- Yusuf Umar
- Daniel Wiberg
- Steven Zhang
- AGreatFish
- BlueMagnificent
- Enhex
- Firegorilla
- Lumak
- Magic.Lixin
- Mike3D
- Modanung
- MonkeyFirst
- Newb I the Newbd
- OvermindDL1
- Skrylar
- TheComet93
- 1vanK
- andmar1x
- amadeus_osa
- atship
- att
- celeron55
- cosmy1
- damu
- dragonCASTjosh
- feltech
- hdunderscore
- lvshiling
- marynate
- mightyCelu
- nemerle
- ninjastone
- raould
- rasteron
- reattiva
- rifai
- skaiware
- ssinai1
- svifylabs
- szamq
- thebluefish
- tommy3
- yushli

Urho3D is greatly inspired by OGRE (http://www.ogre3d.org) and Horde3D
(http://www.horde3d.org). Additional inspiration & research used:
- Rectangle packing by Jukka Jylänki (clb)
  http://clb.demon.fi/projects/rectangle-bin-packing
- Tangent generation from Terathon
  http://www.terathon.com/code/tangent.html
- Fast, Minimum Storage Ray/Triangle Intersection by Möller & Trumbore
  http://www.graphics.cornell.edu/pubs/1997/MT97.pdf
- Linear-Speed Vertex Cache Optimisation by Tom Forsyth
  http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html
- Software rasterization of triangles based on Chris Hecker's
  Perspective Texture Mapping series in the Game Developer magazine
  http://chrishecker.com/Miscellaneous_Technical_Articles
- Networked Physics by Glenn Fiedler
  http://gafferongames.com/game-physics/networked-physics/
- Euler Angle Formulas by David Eberly
  http://www.geometrictools.com/Documentation/EulerAngles.pdf
- Red Black Trees by Julienne Walker
  http://eternallyconfuzzled.com/tuts/datastructures/jsw_tut_rbtree.aspx
- Comparison of several sorting algorithms by Juha Nieminen
  http://warp.povusers.org/SortComparison/

Urho3D uses the following third-party libraries:
- AngelScript 2.30.2 (http://www.angelcode.com/angelscript/)
- Box2D 2.3.0 (http://box2d.org/)
- Bullet 2.83.6 (http://www.bulletphysics.org/)
- Civetweb (http://sourceforge.net/projects/civetweb/)
- FreeType 2.5.0 (http://www.freetype.org/)
- GLEW 1.9.0 (http://glew.sourceforge.net/)
- jo_jpeg 1.52 (http://www.jonolick.com/uploads/7/9/2/1/7921194/jo_jpeg.cpp)
- kNet (https://github.com/juj/kNet)
- libcpuid 0.2.0 (http://libcpuid.sourceforge.net/)
- Lua 5.1 (http://www.lua.org)
- LuaJIT 2.1.0+ (http://www.luajit.org)
- LZ4 (http://code.google.com/p/lz4/)
- MojoShader (http://icculus.org/mojoshader/)
- Mustache 1.0 (http://mustache.github.io/, https://github.com/kainjow/Mustache)
- nanodbc 2.11.3+ (http://lexicalunit.github.io/nanodbc/)
- Open Asset Import Library (http://assimp.sourceforge.net/)
- pugixml 1.5 (http://pugixml.org/)
- rapidjson 0.11 (https://code.google.com/p/rapidjson/)
- Recast/Detour (https://github.com/memononen/recastnavigation/)
- SDL 2.0.4 (http://www.libsdl.org/)
- StanHull (http://codesuppository.blogspot.com/2006/03/
  john-ratcliffs-code-suppository-blog.html)
- stb_image 2.10 (http://nothings.org/)
- stb_image_write 1.01 (http://nothings.org/)
- stb_vorbis 1.07 (http://nothings.org/)
- SQLite 3.8.10.2 (https://www.sqlite.org/)
- tolua++ 1.0.93 (http://www.codenix.com/~tolua)

DXT / ETC1 / PVRTC decompression code based on the Squish library and the Oolong
Engine.
Jack and mushroom models from the realXtend project. (http://www.realxtend.org)
Ninja model and terrain, water, smoke, flare and status bar textures from OGRE.
BlueHighway font from Larabie Fonts.
Anonymous Pro font by Mark Simonson.
NinjaSnowWar sounds by Veli-Pekka Tätilä.

##Documentation
Urho3D classes have been sparsely documented using Doxygen notation. To
generate documentation into the "Docs" subdirectory, open the Doxyfile in the
"Docs" subdirectory with doxywizard and click "Run doxygen" from the "Run" tab.
Get Doxygen from http://www.doxygen.org & Graphviz from http://www.graphviz.org.
See section "Documentation build" below on how to automate documentation
generation as part of the build process.

The documentation is also available online at
  http://urho3d.github.io/documentation/HEAD/index.html

Documentation on how to build Urho3D:
  http://urho3d.github.io/documentation/HEAD/_building.html
Documentation on how to use Urho3D as external library
  http://urho3d.github.io/documentation/HEAD/_using_library.html

Replace HEAD with a specific release version in the above links to obtain the
documentation pertinent to the specified release. Alternatively, use the
document-switcher in the documentation website to do so.

##History
The change history is available online at
  http://urho3d.github.io/documentation/HEAD/_history.html

------------------------------------------------------------


[My logo](https://raw.githubusercontent.com/299299/Urho3D/master/Docs/logo.jpg)

#Android 环境手顺

## Android PC win 7环境配置

  **1. 环境包下载：**

   下载地址 http://pan.baidu.com/s/1jGoZbMm 我的百度共享

  可以选择都安装到比如 F:\Android 目录下

  **2.配置windows环境变量：**

  JAVA_HOME = F:\Android\Java\jdk1.8.0_40

  ANDROID_NDK = F:\Android\android-ndk

  Path变量追加 F:\Android\android-ndk\prebuilt\windows-x86_64\bin;F:\Android\android-sdk\tools;F:\Android\android-sdk\platform-tools;F:\Android\apache-ant-1.9.4\bin

  注意这个是为了能使用 1. ant命令 2. android的make命令

  **3.下载android的SDK Tools：**

  执行Android SDK Manager 根据调试手机选择你要的API Target包

  (Optionally, also install Eclipse ADT plugin for building and deployment via Eclipse.)

  **4.代码编译部署：**

  1. 控制台执行 当前目录下的 make_android.bat (前提是NDK的环境变量都已配好)
  2. cd .build/android ---> 进入目录
  3. android update project -p . -t TARGET_ID  --->  这里TARGET_ID 是通过命令android list targets得到的 我的是android-16
  4. make -j8 --->  第一次编译很慢
  5. ant debug (或者release) ---> .build\android\bin 这里会生成一个apk包 ！
  6. adb install 生成的apk包 就可以部署到手机里了

  **5.代码开发：**

  本身引擎自带anglescript的脚本 但是放到手机上有点大 可以直接写c++的代码

  C++示例可以参考目录 Urho3D/Source/Samples


  其他的平台可以参考 http://urho3d.github.io/documentation/1.32/_building.html


  后续: 因为包太大 可以考虑 删除部分不必要的组件 比如脚本


  这里提供一个android4.2 API编译的测试apk http://pan.baidu.com/s/1sj0qdkD 可以尝试跑一下该引擎以及示例

  第一个游戏例子截图
  ![screen shot](https://raw.githubusercontent.com/299299/Urho3D/master/Docs/shot1.jpg)
