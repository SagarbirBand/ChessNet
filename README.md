# ChessNet
Make Chess Moves between Client and Host
Authored by Sagarbir Bandesha
04/16/24


COMMANDS:

"/move <starting square><endingsquare>" ---------------- ex. "/move e2e4": Moves a piece to its destination if it is a valid rule of chess.

"/chessboard" ------------------------------------------ : Displays the current state of the chessboard for the requested player.

"/save <username> -------------------------------------- ex. "/save Maria": Saves the current state of the game under the username. Both server & client can call.

"/load <username> <int> -------------------------------- ex. "/load Maria 1": Loads a saved game under the username provided and index saved from user

"/import <FEN> ----------------------------------------- ex. "/import 8/8/8/8/8/8/8/8 w": Imports a gamestate given a chess FEN string.

"/forfeit" --------------------------------------------- : Forfeits the game and cuts connection between host & client. The latest FEN is stored in "fen.txt"

NOTES:
1. Invalid moves will require a reprompt.
2. Capture limit is 16 before game terminates.
3. Checks, Checkmates, & King captures should be addressed by the player for the game. These rules are not enforced by ChessNet.
4. Run .\Server.exe & .\Client.exe on windows bash terminals. Best practice to run both commands on same device so ports can be accessed easily.
5. Have fun! Make sure to enjoy & post your games online :)
   
