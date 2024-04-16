#include "ChessNet.h"

void initialize_game(ChessGame *game) 
{
    game->capturedCount = 0; 
    game->currentPlayer = WHITE_PLAYER;
    game->moveCount = 0;

    strcpy(game->chessboard[0], "rnbqkbnr");
    strcpy(game->chessboard[1], "pppppppp");
    strcpy(game->chessboard[2], "........");
    strcpy(game->chessboard[3], "........");
    strcpy(game->chessboard[4], "........");
    strcpy(game->chessboard[5], "........");
    strcpy(game->chessboard[6], "PPPPPPPP");
    strcpy(game->chessboard[7], "RNBQKBNR");
}

void chessboard_to_fen(char fen[], ChessGame *game) 
{
    int fenCt = 0;
    char start = '0';
    int temp = 0;

    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            temp = j;
            while(game->chessboard[i][temp] == '.' && temp < 8)
            {
                start++;
                temp++;
            }
            

            if(start != '0')
            {
                fen[fenCt++] = start;
                j = temp-1;
            }
            else
                fen[fenCt++] = game->chessboard[i][j];
            start = '0';
        }
        if(i != 7)
            fen[fenCt++] = '/';

    }    

    fen[fenCt++] = ' ';

    if(game->currentPlayer == WHITE_PLAYER)
        fen[fenCt++] = 'w';
    else
        fen[fenCt++] = 'b';
    fen[fenCt++] = '\0';

}

bool is_valid_pawn_move(char piece, int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) 
{
    int distance = dest_row - src_row;

    if(piece == 'p' && distance < 0) //moving down means dest must be greater, so we dont violate rule
        return false;
    if(piece == 'P' && distance > 0) //moving up means dest must be lesser, so we dont violate rule
        return false;

    //once we confirm direction with sign, we can make distance positive
    distance = abs(distance);

    //check for case where pawn moves from start square up two spaces
    if(distance == 2)
    {
        if(piece == 'p' && src_row == 1) //black
        {
            if(game->chessboard[src_row+1][src_col] == '.' && game->chessboard[src_row+2][src_col] == '.' && src_col == dest_col)
                return true;
        }

        if(piece == 'P' && src_row == 6) //white
        {
            if(game->chessboard[src_row-1][src_col] == '.' && game->chessboard[src_row-2][src_col] == '.' && src_col == dest_col)
                return true;
        }
    }

    //square must be captureable if moving diagonally.
    if(distance == 1 && abs(dest_col-src_col) == 1)
    {
        if(game->chessboard[dest_row][dest_col] == '.')
            return false;
        return true;
    }

    //generic case for pawn moves
    if(distance == 1 && game->chessboard[dest_row][dest_col] == '.')
        return true;

    return false;
}

bool is_valid_rook_move(int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) 
{

    int temp = 0;
    int inc = 0;

    bool rk = false;

    if(src_col == dest_col)
    {
        rk = true;
        if(dest_row > src_row)
            inc = 1;
        else
            inc = -1;

        temp = src_row + inc;

        //Always do one less than expected so we do not have to test piece captured as separate. We assume destination is capturable or vacant
        for(int i = 0; i < abs(dest_row-src_row)-1; i++)
        {
            if(game->chessboard[temp][src_col] != '.')
                rk = false;
            temp += inc;
        }
    }

    if(src_row == dest_row)
    {
        rk = true;
        if(dest_col > src_col)
            inc = 1;
        else
            inc = -1;

        temp = src_col + inc;

        //Always do one less than expected so we do not have to test piece captured as separate. We assume destination is capturable or vacant
        for(int i = 0; i < abs(dest_col-src_col)-1; i++)
        {
            if(game->chessboard[src_row][temp] != '.')
                rk = false;
            temp += inc;
        }

    }
    return rk;
}

bool is_valid_knight_move(int src_row, int src_col, int dest_row, int dest_col) 
{

    //summation of row change & column change must be 3
    int rowDiff = abs(dest_row-src_row);
    int colDiff = abs(dest_col-src_col);

    if(colDiff + rowDiff == 3)
        return true;

    return false;
}

bool is_valid_bishop_move(int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) 
{

    if(src_col == dest_col || src_row == dest_row)
        return false;

    //check if diagonals can even be achieved
    int currCol = src_col;
    int currRow = src_row;
    int verticalInc = 0;
    int horizInc = 0;

    if(src_row < dest_row)
        verticalInc = 1;
    else
        verticalInc = -1;

    if(src_col < dest_col)
        horizInc = 1;
    else
        horizInc = -1;
    
    while(currRow != dest_row)
    {
        currCol += horizInc;
        currRow += verticalInc;
    }

    if(currCol != dest_col && currRow != dest_row)
        return false;
    
    //reset this to check for pieces inbetween
    currCol = src_col + horizInc;
    currRow = src_row + verticalInc;

    bool bishop = true;

    for(int i = 0; i < abs(src_row-dest_row)-1; i++)
    {
        if(game->chessboard[currRow][currCol] != '.')
            bishop = false;
        currRow += verticalInc;
        currCol += horizInc;
    }

    return bishop;

}

bool is_valid_queen_move(int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) 
{
    bool rookVal = is_valid_rook_move(src_row, src_col, dest_row, dest_col, game);
    bool bishopVal = is_valid_bishop_move(src_row, src_col, dest_row, dest_col, game);

    if(rookVal || bishopVal)
        return true;

    return false;
}

bool is_valid_king_move(int src_row, int src_col, int dest_row, int dest_col) 
{
    int vertDist = abs(src_row-dest_row);
    int horizDist = abs(src_col-dest_col);

    if(vertDist < 2 && horizDist < 2)
        return true;

    return false;
}

bool is_valid_move(char piece, int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) 
{
    
    bool vMove = false;

    switch (piece)
    {
        case 'p':
        case 'P':
            vMove = is_valid_pawn_move(piece, src_row, src_col, dest_row, dest_col, game);
            break;
        case 'r':
        case 'R':
            vMove = is_valid_rook_move(src_row, src_col, dest_row, dest_col, game);
            break;
        case 'n':
        case 'N':
            vMove = is_valid_knight_move(src_row, src_col, dest_row, dest_col);
            break;
        case 'b':
        case 'B':
            vMove = is_valid_bishop_move(src_row, src_col, dest_row, dest_col, game);
            break;
        case 'q':
        case 'Q':
            vMove = is_valid_queen_move(src_row, src_col, dest_row, dest_col, game);
            break;
        case 'k':
        case 'K':
            vMove = is_valid_king_move(src_row, src_col, dest_row, dest_col);
            break;
        
        default:
            break;
    }
    return vMove;
}

int parse_move(const char *move, ChessMove *parsed_move) 
{
    if(strlen(move) != 4 && strlen(move) != 5)
        return PARSE_MOVE_INVALID_FORMAT;

    parsed_move->startSquare[0] = move[0];
    parsed_move->startSquare[1] = move[1];
    parsed_move->startSquare[2] = '\0';

    parsed_move->endSquare[0] = move[2];
    parsed_move->endSquare[1] = move[3];
    parsed_move->endSquare[2] = '\0';
    parsed_move->endSquare[3] = '\0';
    if(strlen(move) == 5)
        parsed_move->endSquare[2] = move[4];
    
    if(parsed_move->startSquare[0] < 'a' || parsed_move->startSquare[0] > 'h' || parsed_move->endSquare[0] < 'a' || parsed_move->endSquare[0] > 'h')
        return PARSE_MOVE_INVALID_FORMAT;

    if(parsed_move->startSquare[1] < '1' || parsed_move->startSquare[1] > '8' || parsed_move->endSquare[1] < '1' || parsed_move->endSquare[1] > '8')
        return PARSE_MOVE_OUT_OF_BOUNDS;

    if(strlen(move) == 5)
    {
        if(!(parsed_move->endSquare[1] == '1' || parsed_move->endSquare[1] == '8'))
            return PARSE_MOVE_INVALID_DESTINATION;
        if(!(parsed_move->endSquare[2] == 'q' || parsed_move->endSquare[2] == 'r' || parsed_move->endSquare[2] == 'b' || parsed_move->endSquare[2] == 'n'))
            return PARSE_MOVE_INVALID_PROMOTION;
    }
    
    return 0;

}

int make_move(ChessGame *game, ChessMove *move, bool is_client, bool validate_move) 
{

    //converting move to usable sources and destinations
    int src_col = move->startSquare[0] - 97; // a - h
    int src_row = 7 - (move->startSquare[1] - 49); // 1 - 8
    int dest_col = move->endSquare[0] - 97; // a - h
    int dest_row = 7 - (move->endSquare[1] - 49); // 1 - 8

    char piece = game->chessboard[src_row][src_col];

    if(validate_move)
    {
        //correct person making move
        if(game->currentPlayer == WHITE_PLAYER && is_client == false)
            return MOVE_OUT_OF_TURN;
        if(game->currentPlayer == BLACK_PLAYER && is_client == true)
            return MOVE_OUT_OF_TURN;

        //moving nothing
        if(game->chessboard[src_row][src_col] == '.')
            return MOVE_NOTHING;

        //wrong color move from opponent
        if(game->currentPlayer == WHITE_PLAYER && game->chessboard[src_row][src_col] > 'a' && game->chessboard[src_row][src_col] < 'z')
            return MOVE_WRONG_COLOR;
        if(game->currentPlayer == BLACK_PLAYER && game->chessboard[src_row][src_col] > 'A' && game->chessboard[src_row][src_col] < 'Z')
            return MOVE_WRONG_COLOR;

        //capturing own piece
        if(game->currentPlayer == WHITE_PLAYER && (game->chessboard[dest_row][dest_col] > 'A' && game->chessboard[dest_row][dest_col] < 'Z'))
            return MOVE_SUS;
        if(game->currentPlayer == BLACK_PLAYER && (game->chessboard[dest_row][dest_col] > 'a' && game->chessboard[dest_row][dest_col] < 'z'))
            return MOVE_SUS;

        //if piece at source is not a pawn
        if(move->endSquare[2] != '\0' && (game->chessboard[src_row][src_col] != 'p' && game->chessboard[src_row][src_col] != 'P'))
            return MOVE_NOT_A_PAWN;
        
        //if piece is pawn and destination is edge rows without srlen 5 of move
        if((piece == 'p' || piece == 'P') && (dest_row == 0 || dest_row == 7) && move->endSquare[2] == '\0')
            return MOVE_MISSING_PROMOTION;
        
        //check if move is valid
        if(is_valid_move(piece, src_row, src_col, dest_row, dest_col, game) == false)
            return MOVE_WRONG;
    }

    //append move to game->moves
    game->moves[game->moveCount] = *move;
    game->moveCount++;

    //make current square vacant
    game->chessboard[src_row][src_col] = '.';

    //if piece on square captured, append to captured pieces
    if(game->chessboard[dest_row][dest_col] != '.')
    {
        game->capturedPieces[game->capturedCount] = game->chessboard[dest_row][dest_col];
        game->capturedCount++;
    }

    //make dest square the piece which moved
    game->chessboard[dest_row][dest_col] = piece;


    //if there is a promotion, convert the pawn accordingly
    if(move->endSquare[2] != '\0')
    {
        if(game->currentPlayer == WHITE_PLAYER)
            game->chessboard[dest_row][dest_col] = toupper(move->endSquare[2]);
        else
            game->chessboard[dest_row][dest_col] = move->endSquare[2]; 
    }
    
    //update current player to opposite of who just played
    if(game->currentPlayer == WHITE_PLAYER)
        game->currentPlayer = BLACK_PLAYER;
    else
        game->currentPlayer = WHITE_PLAYER;

    return 0;
}

void fen_to_chessboard(const char *fen, ChessGame *game) 
{
    
    int fenSize = strlen(fen);
    int currRow = 0;
    int currCol = 0;
    char player;

    memset(game->chessboard, '.', sizeof(char) * 8 * 8); //make everything blank so digits are easier to parse

    for(int i = 0; i < fenSize-1; i++)
    {
        if(fen[i] == '/')
            currRow++;
        else if (fen[i] > '0' && fen[i] <= '9')
            currCol += (fen[i] - 48);
        else
            game->chessboard[currRow][currCol++] = fen[i];

        if(currCol == 8 && currRow != 7)
            currCol = 0;

        if(currCol == 8 && currRow && 7)
        {
            player = fen[i+2];
            break;
        }
    }

    if (player == 'w')
        game->currentPlayer = WHITE_PLAYER;
    else
        game->currentPlayer = BLACK_PLAYER;
    
}


int send_command(ChessGame *game, const char *msg, int socketfd, bool is_client) 
{
    char* cmd;
    int misc;

    char* message = strdup(msg);

    cmd = strtok(message, " ");

    if(strcmp(message, "/forfeit") == 0)
    {
        send(socketfd, msg, strlen(msg), 0);
        return COMMAND_FORFEIT;
    }
    
    if(strcmp(message, "/chessboard") == 0)
    {
        display_chessboard(game);
        return COMMAND_DISPLAY;
    }

    if(strcmp(message, "/save") == 0)
    {
        cmd = strtok(NULL, " ");
        save_game(game, cmd, "game_database.txt");
        return COMMAND_SAVE;
    }

    if(strcmp(message, "/move") == 0)
    {
        const char* temp = strtok(NULL, " ");
        ChessMove parsed_move;
        misc = parse_move(temp, &parsed_move);
        if(misc == 0)
            misc = make_move(game, &parsed_move, is_client, true);
        else
            return COMMAND_ERROR;
        
        if(misc == 0)
            send(socketfd, msg, strlen(msg), 0);       
        else
            return COMMAND_ERROR;
        
        return COMMAND_MOVE;

    }

    if(strcmp(message, "/import") == 0)
    {
        if(is_client)
        {
            cmd = strtok(NULL, " ");
            fen_to_chessboard(cmd, game);
            send(socketfd, msg, strlen(msg), 0);
            return COMMAND_IMPORT;
        }
        return COMMAND_ERROR;
    }

    if(strcmp(message, "/load") == 0)
    {
        cmd = strtok(NULL, " ");
        misc = atoi(strtok(NULL, " "));
        misc = load_game(game, cmd, "game_database.txt", misc);
        if (misc == -1)
            return COMMAND_ERROR;
        
        send(socketfd, msg, strlen(msg), 0);
        return COMMAND_LOAD;
    }


    return COMMAND_UNKNOWN;
}

int receive_command(ChessGame *game, const char *msg, int socketfd, bool is_client) {
    char* cmd;
    char* t;
    int misc;
    (void)socketfd;

    char* message = strdup(msg);

    cmd = strtok(message, " ");

    if(strcmp(message, "/forfeit") == 0)
        return COMMAND_FORFEIT;

    if(strcmp(message, "/move") == 0)
    {
        const char* temp = strtok(NULL, " ");
        ChessMove parsed_move;
        misc = parse_move(temp, &parsed_move);
        if(misc == 0)
            misc = make_move(game, &parsed_move, is_client, false);
        else
            return COMMAND_ERROR;

        if(misc != 0)
            return COMMAND_ERROR;
        
        return COMMAND_MOVE;

    }

    if(strcmp(message, "/import") == 0)
    {
        cmd = strtok(NULL, "");
        if(is_client)
            fen_to_chessboard(cmd, game);
        else
            return COMMAND_ERROR;

        return COMMAND_IMPORT;
    }

    if(strcmp(message, "/load") == 0)
    {
        cmd = strtok(NULL, " ");
        t = strtok(NULL, " ");
        if(t != NULL && isdigit(t[0]))
            misc = atoi(t);
        else
            return COMMAND_ERROR;

        misc = load_game(game, cmd, "game_database.txt", misc);
        if (misc == -1)
            return COMMAND_ERROR;

        return COMMAND_LOAD;
    }

    return -1;
}

int save_game(ChessGame *game, const char *username, const char *db_filename) 
{
    if(strlen(username) < 1)
        return -1;

    const char *ptr = username;
    while(*ptr != '\0')
    {
        if(*ptr == ' ')
            return -1;
        ptr++;
    }

    FILE* file = fopen(db_filename, "a");
    if(file == NULL)
        return -1;

    char fen[BUFFER_SIZE];
    chessboard_to_fen(fen, game);

    int len = strlen(username) + strlen(fen) + 3;

    char total[len];
    strcpy(total, username);
    strcat(total, ":");
    strcat(total, fen);
    strcat(total, "\n");

    fputs(total, file);
    fclose(file);

    return 0;


}

int load_game(ChessGame *game, const char *username, const char *db_filename, int save_number)
{
    int count = 0;
    FILE* file = fopen(db_filename, "r");
    if (file == NULL)
        return -1;

    char line[BUFFER_SIZE];
    char* user;
    char* fen;

    while(fgets(line, sizeof(line), file))
    {
        user = strtok(line, ":");
        if(user != NULL && strcmp(user, username) == 0)
        {
            count++;
            if(count == save_number)
            {
                fen = strtok(NULL, ":");
                fen_to_chessboard(fen, game);
                return 0;
            } 
        }
    }

    fclose(file);  
    return -1;

}

void display_chessboard(ChessGame *game) {
    printf("\nChessboard:\n");
    printf("  a b c d e f g h\n");
    for (int i = 0; i < 8; i++) {
        printf("%d ", 8 - i);
        for (int j = 0; j < 8; j++) {
            printf("%c ", game->chessboard[i][j]);
        }
        printf("%d\n", 8 - i);
    }
    printf("  a b c d e f g h\n");
}
