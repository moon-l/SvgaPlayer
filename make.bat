set SHADOWPATH=%~dp0shadow
mkdir %SHADOWPATH%
cd /d %SHADOWPATH%
qmake -tp vc -r ../svgaplayer.pro
cd ..