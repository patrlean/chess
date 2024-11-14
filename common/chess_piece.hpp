// chess_piece.hpp
#ifndef CHESS_PIECE_HPP
#define CHESS_PIECE_HPP

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// add enum definition in chess_piece.hpp
enum class ChessPieceType {
    PAWNBLACK,
    ROOKBLACK,
    KNIGHTBLACK,
    BISHOPBLACK,
    QUEENBLACK,
    KINGBLACK,
    PAWNWHITE,
    ROOKWHITE,
    KNIGHTWHITE,
    BISHOPWHITE,
    QUEENWHITE,
    KINGWHITE
};
// add type field in ChessPiece structure
struct ChessPiece {
    
    std::string name;
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint elementbuffer;
    
    glm::vec3 position;
    glm::vec3 scale;
    float rotationY;
    
    ChessPiece();
    glm::mat4 getTransform() const;
    ChessPieceType type;
    bool isBlack;  // true for black pieces, false for white pieces
    
    // add helper function to get type name
    std::string getTypeName() const {
        switch(type) {
            case ChessPieceType::PAWNBLACK:
            case ChessPieceType::PAWNWHITE:
                return "Pawn";
            case ChessPieceType::ROOKBLACK:
            case ChessPieceType::ROOKWHITE:
                return "Rook";
            case ChessPieceType::KNIGHTBLACK:
            case ChessPieceType::KNIGHTWHITE:
                return "Knight";
            case ChessPieceType::BISHOPBLACK:
            case ChessPieceType::BISHOPWHITE:
                return "Bishop";
            case ChessPieceType::QUEENBLACK:
            case ChessPieceType::QUEENWHITE:
                return "Queen";
            case ChessPieceType::KINGBLACK:
            case ChessPieceType::KINGWHITE:
                return "King";
            default:
                return "Unknown";
        }
    }
};

class ChessboardHelper {
public:
    static constexpr float SQUARE_SIZE = 5.4f;        // size of each square
    static constexpr float BOARD_OFFSET_X = -16.0f;    // offset of the left edge of the board
    static constexpr float BOARD_OFFSET_Z = -22.0f;    // offset of the top edge of the board
    static constexpr float PIECE_HEIGHT = 2.0f;       // height of the piece on the board

    static glm::vec3 getSquarePosition(int row, int col) {
        float x = BOARD_OFFSET_X + col * SQUARE_SIZE;
        float z = BOARD_OFFSET_Z + row * SQUARE_SIZE;
        float y = PIECE_HEIGHT;  // height of the piece on the board
        return glm::vec3(x, y, z);
    }
};

glm::vec3 calculatePieceCenter(const std::vector<glm::vec3>& vertices);
void movePieceToPosition(std::vector<glm::vec3>& vertices, const glm::vec3& currentCenter, const glm::vec3& targetPosition);

void initializeChessPieces(std::vector<ChessPiece>& pieces);
void renderChessPieces(const std::vector<ChessPiece>& pieces,
                      const glm::mat4& ProjectionMatrix,
                      const glm::mat4& ViewMatrix,
                      GLuint MatrixID,
                      GLuint ModelMatrixID,
                      GLuint ViewMatrixID);
void movePiece(ChessPiece& piece, int toRow, int toCol);

void createGLBuffers(ChessPiece& piece);

#endif // CHESS_PIECE_H