@echo off
rem ============================================================================
rem  empaquetar.bat - Arma la carpeta "HogarBotanico3D" lista para InstallForge.
rem
rem  Como usarlo (en Windows):
rem    1. En Visual Studio compila en  Release | x64  (Compilar > Recompilar solucion).
rem       El Proyecto.exe debe quedar en esta misma carpeta (Proyecto\Proyecto).
rem    2. Haz doble clic en este archivo.
rem    3. Al terminar, la carpeta  ..\Paquete\HogarBotanico3D  tiene todo lo necesario.
rem       Probar el programa ahi (doble clic en Proyecto.exe) ANTES de hacer el instalador.
rem ============================================================================
setlocal EnableExtensions
chcp 65001 >nul
cd /d "%~dp0"

set "ORIGEN=%~dp0"
set "DESTINO=%~dp0..\Paquete\HogarBotanico3D"

echo.
echo === Empaquetando Hogar Botanico 3D ===
echo.

rem --- 1. Revisar que exista el ejecutable (compilado en Release)
if not exist "%ORIGEN%Proyecto.exe" (
    echo [ERROR] No se encontro Proyecto.exe en esta carpeta.
    echo         Compila primero en Visual Studio con configuracion Release x64.
    pause
    exit /b 1
)

rem --- 2. Carpeta limpia
if exist "%DESTINO%" rmdir /s /q "%DESTINO%"
mkdir "%DESTINO%"

rem --- 3. Ejecutable y bibliotecas (DLL) que necesita
copy /y "%ORIGEN%Proyecto.exe"          "%DESTINO%\" >nul
copy /y "%ORIGEN%assimp-vc143-mt.dll"   "%DESTINO%\" >nul || echo [AVISO] Falta assimp-vc143-mt.dll
copy /y "%ORIGEN%glew32.dll"            "%DESTINO%\" >nul || echo [AVISO] Falta glew32.dll
copy /y "%ORIGEN%irrKlang.dll"          "%DESTINO%\" >nul || echo [AVISO] Falta irrKlang.dll
copy /y "%ORIGEN%ikpMP3.dll"            "%DESTINO%\" >nul || echo [AVISO] Falta ikpMP3.dll
copy /y "%ORIGEN%ikpFlac.dll"           "%DESTINO%\" >nul || echo [AVISO] Falta ikpFlac.dll
copy /y "%ORIGEN%zlib.dll"              "%DESTINO%\" >nul || echo [AVISO] Falta zlib.dll

rem --- 4. Modelos: casa y personaje (con sus carpetas de texturas .fbm)
copy /y "%ORIGEN%Casa_coliciones3.fbx"        "%DESTINO%\" >nul || echo [AVISO] Falta Casa_coliciones3.fbx
copy /y "%ORIGEN%personaje_estudiante2.fbx"   "%DESTINO%\" >nul || echo [AVISO] Falta personaje_estudiante2.fbx
robocopy "%ORIGEN%Casa_coliciones3.fbm"          "%DESTINO%\Casa_coliciones3.fbm"          /E /NFL /NDL /NJH /NJS /NP >nul
robocopy "%ORIGEN%Casa_coliciones2.fbm"          "%DESTINO%\Casa_coliciones2.fbm"          /E /NFL /NDL /NJH /NJS /NP >nul
robocopy "%ORIGEN%personaje_estudiante_fix.fbm"  "%DESTINO%\personaje_estudiante_fix.fbm"  /E /NFL /NDL /NJH /NJS /NP >nul

rem --- 5. Carpetas de recursos: shaders, texturas, muebles (con tarjetas y pantalla de inicio) y minijuegos
robocopy "%ORIGEN%shaders"  "%DESTINO%\shaders"  /E /NFL /NDL /NJH /NJS /NP >nul
robocopy "%ORIGEN%textures" "%DESTINO%\textures" /E /NFL /NDL /NJH /NJS /NP /XF *.zip >nul
robocopy "%ORIGEN%muebles"  "%DESTINO%\muebles"  /E /NFL /NDL /NJH /NJS /NP >nul
robocopy "%ORIGEN%minijuegos" "%DESTINO%\minijuegos" /E /NFL /NDL /NJH /NJS /NP >nul
rem     (minijuegos = pin.png de los checkpoints y las imagenes de la trivia)

rem --- 6. Manual (si ya existe)
if exist "%ORIGEN%manual.pdf" copy /y "%ORIGEN%manual.pdf" "%DESTINO%\" >nul

rem --- 7. Biblioteca de Visual C++ (para que funcione en equipos sin Visual Studio)
rem     Se busca en las carpetas habituales de Visual Studio 2022 y se copia junto al .exe.
set "CRT_ENCONTRADO="
for /d %%E in ("%ProgramFiles%\Microsoft Visual Studio\2022\*") do (
    for /d %%V in ("%%E\VC\Redist\MSVC\*") do (
        if exist "%%V\x64\Microsoft.VC143.CRT\vcruntime140.dll" (
            if not defined CRT_ENCONTRADO (
                copy /y "%%V\x64\Microsoft.VC143.CRT\*.dll" "%DESTINO%\" >nul
                set "CRT_ENCONTRADO=%%V"
            )
        )
    )
)
if defined CRT_ENCONTRADO (
    echo [OK] Biblioteca de Visual C++ copiada desde: %CRT_ENCONTRADO%
) else (
    echo [AVISO] No se encontro la biblioteca de Visual C++. Si en otra computadora sale un error
    echo         de vcruntime140.dll o msvcp140.dll, instala "Visual C++ Redistributable x64" ahi.
)

echo.
echo === Listo ===
echo Carpeta creada en: %DESTINO%
echo.
echo SIGUIENTE PASO: abre %DESTINO%\Proyecto.exe y confirma que todo funciona.
echo Despues, esa misma carpeta se usa en InstallForge para crear el setup.exe.
echo.
pause
endlocal
