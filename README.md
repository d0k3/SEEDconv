# SEEDconv
seeddb.bin generator for the 3DS console

Instructions:

Get the file at nand:/data/<console-unique>/sysdata/0001000f/00000000 (only available on FW 9.6, thus only available with a hardmod). Copy this to some directory on your PC together with seedconv.exe. Then just drag and drop 00000000 (or multiple different 00000000 files) onto seedconv.exe, and a working seeddb.bin should automagically be generated.

The seeddb.bin file is compatible with up to date Decrypt9 - see here: https://github.com/archshift/Decrypt9 or here: https://github.com/d0k3/Decrypt9/releases (experimental release with Ninjhax loader included).
