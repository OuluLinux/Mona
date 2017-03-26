# Mona

This is a fork of Mona neural network library, which converts it to use U++ library for the core and examples.

The conversion is currently ongoing, and has following steps:
1. Move original code to target file structure, using TheIDE
2. Convert code to use U++ conventions (upgrade old style) and to use U++ functions.
3. Convert examples to use CtrlLib instead of GLUT for GUI.
4. Add fast-forward, plotters and other debug drawers.
5. Use code in few own examples.
