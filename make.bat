set SHADOWPATH=%~dp0shadow
mkdir %SHADOWPATH%
set ProtoCmd=%~dp03rd/protobuf-3.3.0/bin/protoc.exe
set ProtoPath=%~dp0svgaplayer/proto
"%ProtoCmd%" --proto_path=%ProtoPath% --cpp_out=%ProtoPath% %ProtoPath%/svga.proto
cd /d %SHADOWPATH%
qmake -tp vc -r ../svgaplayer.pro
cd ..