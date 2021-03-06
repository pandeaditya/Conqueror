Conqueror
=========

A chess engine written in C++. It currently implements hybrid board representation, alphabeta pruning, quiescence search, static evaluation, legal move generator, iterative deepening, MVV/LVA move reordering, makemove, unmakemove and uci move representation standard, tapered evaluation. Elostat gives the current version an ELO of 2100 measured over 50 games.
The binary released supports UCI and can be used with any UCI supporting GUI. However, I suggest using Arena GUI (get the latest version from http://www.playwitharena.com/) or latest Winboard(http://www.open-aurec.com/wbforum/viewtopic.php?t=51528).

Easy Install:
=============

1. Goto https://sourceforge.net/projects/conqueror-chess-engine/files/
2. Download any version you are more comfortable unzipping or the installer that will automate the following steps
3. Unzip to a suitable location
4. Start Winboard.exe and enjoy
5. In case of Installer.exe, just run it from the desktop shortcut

Installation With Arena
=======================

1. Install Arena GUI
2. Goto engines -> Install new engine
3. Select Conqueror.exe from the dialog box
4. Goto engine -> manage (F11)
5. Select Conqueror as the current chess engine
6. Enjoy
7. (Optional) Send your queries, comments, complaints or suggestions to pandeaditya7@gmail.com

Installation With Winboard
==========================

1. Install Winboard 4.7
2. Goto the Winboard installation directory and add a new folder Conqueror and put the Conqueror.exe release from this repo inside it
3. Goto engine -> edit engine list
4. Append the line "Conqueror" -fcp "Conqueror" -fd "..\Conqueror" -fn "Conqueror" -fUCI /firstXBook
5. Close winboard and reopen it
6. Goto engine -> Load first engine
7. Select Conqueror from the list
8. Enjoy
