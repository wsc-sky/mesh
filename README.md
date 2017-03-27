//Mesh Viewer Instruction

Compiling platform: Mac OS X

Programming tool: xcode7.3

Third party libraries: 
glm: use some function of glm: vector, cross(), normalize().

nfd: use nfd to select a .m file in a dialogue.

How to run and compile(please use OS X): 

First open my project file: mesh_project, 

The project in xcode must include these frameworks:

GLUI.framework, OpenGL.framework, GLUT.framework,Foundation.famework,AppKit.framework

Remark: the last two are use to nfd

you can run the project immediately in: /build/bin/mesh_view.exec

Or you can open the project in xcode in: /build/project/ mesh_view.xcodeproj

My kernel code is in /test/ mesh.cpp, and other code files are all from third libraries, please do not change them.
