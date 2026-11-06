# Cómo crear el instalador (setup.exe) de Hogar Botánico 3D

Todo esto se hace en **Windows**, con Visual Studio e InstallForge.

## 1. Compilar en Release
1. Abre `Proyecto.slnx` en Visual Studio 2022.
2. Arriba elige **Release** y **x64** (no Debug).
3. Menú **Compilar → Recompilar solución**. Debe terminar sin errores.
4. Verifica que exista `Proyecto\Proyecto\Proyecto.exe`.

## 2. Armar la carpeta del programa
1. Doble clic en `empaquetar.bat` (está junto a `Proyecto.exe`).
2. Se crea `Proyecto\Paquete\HogarBotanico3D\` con el .exe, las DLL, shaders, texturas, muebles y modelos.
3. Si el script muestra algún `[AVISO]`, ese archivo falta; avísame cuál.

## 3. Probar antes del instalador
1. Entra a `Paquete\HogarBotanico3D` y abre `Proyecto.exe` con doble clic.
2. Revisa: sale la pantalla de inicio, entra a la casa, hay muebles y plantas, se ven las texturas,
   el personaje camina con la tecla P (tercera persona) y funciona el clic en plantas y puertas.
3. Si algo falta o se ve negro, **no hagas el instalador todavía**.

## 4. Crear el instalador con InstallForge
InstallForge es gratuito: https://installforge.net (Windows). Los nombres de los menús pueden variar un poco
según la versión; la idea es la siguiente:

1. **General:** nombre de la aplicación `Hogar Botánico 3D`, versión `1.0`, empresa/autor con los nombres del equipo.
2. **Setup → Files (Archivos):** agrega **toda la carpeta** `Paquete\HogarBotanico3D`
   (opción "Add Folder", incluyendo subcarpetas). Destino: la carpeta de instalación del programa.
3. **Directorio de instalación por defecto:** `Program Files\Hogar Botánico 3D` (o una carpeta en el Escritorio).
4. **Accesos directos:** crear uno en el Escritorio y en el Menú Inicio que apunten a `Proyecto.exe`.
   Poner la carpeta de instalación como "Directorio de inicio" del acceso directo (importante: el programa
   busca shaders, texturas y modelos desde ahí).
5. **Build (Compilar):** elige dónde guardar el resultado y genera `setup.exe`.

## 5. Probar el setup.exe
1. Cópialo a otra carpeta o a otra computadora y ejecútalo.
2. Instala, abre el acceso directo y confirma que el recorrido funciona igual.
3. Si sale un error de `vcruntime140.dll` o `msvcp140.dll`, instala "Visual C++ Redistributable x64" en ese equipo.

## Si el programa abre pero se ve vacío o negro
Casi siempre es que el programa no encuentra sus archivos: debe ejecutarse **con su carpeta como directorio de trabajo**
(por eso el paso del "Directorio de inicio" en el acceso directo).
