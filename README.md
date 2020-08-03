# osgRmlUi
Integration of RmlUi with OpenSceneGraph

This project is a resurrection of osgLibRocket to work with RmlUi-3.3 and OpenSceneGraph-3.6.5.

It uses cmake to create the Makefiles or other build system.

The RenderInterface works by creating an osg::Geometry for each call to CompileGeometry() and returning a handle for later use by RenderGeometry() and RenderCompiledGeometry().

The following dependencies are known to work:
  * cmake-3.16.3 - The use of cmake is quite simple, so it is likely that earlier versions will work too.
  * OpenSceneGraph-3.6.5 - Again, it is quite possible that earlier versions will work.
  * RmlUi-3.3 - It is unknown whether earlier versions will work.  There is ongoing work to restructure the code for RmlUi which will have implications for osgRmlUi.

To build the code on Linux:
* git clone https://github.com/triblatron/osgRmlUi.git
* mkdir osgRmlUi_build
* cd osgRmlUi_build
* cmake ../osgRmlUi
* ccmake or cmake-gui and edit the include paths and libraries for OpenSceneGraph and OpenThreads.
* configure
* generate
make
