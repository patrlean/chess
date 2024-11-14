// chess_piece.cpp
#include "chess_piece.hpp"
#include <algorithm>
#include <map>
#include <iostream>

ChessPiece::ChessPiece() : 
    position(0.0f),
    scale(glm::vec3(0.02f)),
    rotationY(0.0f)
{}



glm::mat4 ChessPiece::getTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::scale(transform, scale);
    return transform;
}       

// compute chess piece center
glm::vec3 calculatePieceCenter(const std::vector<glm::vec3>& vertices) {
    glm::vec3 min(FLT_MAX, 0.0f, FLT_MAX); 
    glm::vec3 max(-FLT_MAX, 0.0f, -FLT_MAX); 
    
    for (const auto& vertex : vertices) {
        min.x = std::min(min.x, vertex.x);
        min.z = std::min(min.z, vertex.z);
        
        max.x = std::max(max.x, vertex.x);
        max.z = std::max(max.z, vertex.z);
    }
    
    return glm::vec3((min.x + max.x) * 0.5f, 0.0f, (min.z + max.z) * 0.5f); // set y to 0
}

// move chess piece to target position
void movePieceToPosition(std::vector<glm::vec3>& vertices, const glm::vec3& currentCenter, const glm::vec3& targetPosition) {
    glm::vec3 offset = targetPosition - currentCenter;
    for (auto& vertex : vertices) {
        vertex += offset;
    }
}

void initializeChessPieces(std::vector<ChessPiece>& pieces) {
    if (pieces.empty()) return;

    std::cout << "\n=== Initializing Chess Pieces ===\n" << std::endl;

    // step 1: move all chess pieces to the top-left corner of the board
    glm::vec3 boardTopLeft(ChessboardHelper::BOARD_OFFSET_X,  // x = -8.0f
                          ChessboardHelper::PIECE_HEIGHT,      // y = 2.5f
                          ChessboardHelper::BOARD_OFFSET_Z);   // z = 8.0f

    for (auto& piece : pieces) {
        // compute current chess piece center
        glm::vec3 currentCenter = calculatePieceCenter(piece.vertices);
        std::cout << piece.name << " original center: (" 
                  << currentCenter.x << ", " << currentCenter.y << ", " 
                  << currentCenter.z << ")" << std::endl;

        // move to the top-left corner of the board
        movePieceToPosition(piece.vertices, currentCenter, boardTopLeft);
        
        std::cout << piece.name << " moved to board top-left corner ("
                  << boardTopLeft.x << ", " << boardTopLeft.y << ", "
                  << boardTopLeft.z << ")" << std::endl;
    }

    // save original chess pieces as templates
    std::vector<ChessPiece> templatePieces = pieces;
    pieces.clear();

    // define initial positions of chess pieces
    struct PiecePosition {
        ChessPieceType type;
        int count;
        int row;
        std::vector<int> columns;
    };

    // define initial positions of black chess pieces
    const std::vector<PiecePosition> blackPieces = {
        {ChessPieceType::ROOKBLACK,   2, 0, {0, 7}},
        {ChessPieceType::KNIGHTBLACK, 2, 0, {1, 6}},
        {ChessPieceType::BISHOPBLACK, 2, 0, {2, 5}},
        {ChessPieceType::QUEENBLACK,  1, 0, {3}},
        {ChessPieceType::KINGBLACK,   1, 0, {4}},
        {ChessPieceType::PAWNBLACK,   8, 1, {0, 1, 2, 3, 4, 5, 6, 7}}
    };

    // define initial positions of white chess pieces
    const std::vector<PiecePosition> whitePieces = {
        {ChessPieceType::ROOKWHITE,   2, 7, {0, 7}},
        {ChessPieceType::KNIGHTWHITE, 2, 7, {1, 6}},
        {ChessPieceType::BISHOPWHITE, 2, 7, {2, 5}},
        {ChessPieceType::QUEENWHITE,  1, 7, {3}},
        {ChessPieceType::KINGWHITE,   1, 7, {4}},
        {ChessPieceType::PAWNWHITE,   8, 6, {0, 1, 2, 3, 4, 5, 6, 7}}
    };

    // process each template chess piece
    for (const auto& templatePiece : templatePieces) {
        // find corresponding position configuration
        const auto& positions = templatePiece.isBlack ? blackPieces : whitePieces;
        
        for (const auto& pos : positions) {
            if (pos.type == templatePiece.type) {
                for (size_t i = 0; i < pos.columns.size(); ++i) {
                    ChessPiece newPiece = templatePiece;
                    newPiece.name = (newPiece.isBlack ? "Black " : "White ") + 
                                  newPiece.getTypeName() + " " + 
                                  std::to_string(i + 1);

                    // compute target position relative to the top-left corner of the board
                    float offsetX = pos.columns[i] * ChessboardHelper::SQUARE_SIZE;
                    float offsetZ = pos.row * ChessboardHelper::SQUARE_SIZE;

                    // move to target position
                    glm::vec3 targetPosition = glm::vec3(
                        boardTopLeft.x + offsetX,
                        boardTopLeft.y,
                        boardTopLeft.z + offsetZ
                    );

                    // move vertices to target position
                    movePieceToPosition(newPiece.vertices, boardTopLeft, targetPosition);
                    
                    // set final position (for transformation matrix)
                    newPiece.position = targetPosition;
                    
                    // set rotation (black and white pieces face each other)
                    newPiece.rotationY = newPiece.isBlack ? 0.0f : 180.0f;

                    pieces.push_back(newPiece);
                }
                break;
            }
        }
    }

    // generate OpenGL buffers for all chess pieces
    for (auto& piece : pieces) {
        glGenBuffers(1, &piece.vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, piece.vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, piece.vertices.size() * sizeof(glm::vec3), &piece.vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &piece.uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, piece.uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, piece.uvs.size() * sizeof(glm::vec2), &piece.uvs[0], GL_STATIC_DRAW);

        glGenBuffers(1, &piece.normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, piece.normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, piece.normals.size() * sizeof(glm::vec3), &piece.normals[0], GL_STATIC_DRAW);

        glGenBuffers(1, &piece.elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, piece.elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, piece.indices.size() * sizeof(unsigned short), &piece.indices[0], GL_STATIC_DRAW);
    }
}

void renderChessPieces(const std::vector<ChessPiece>& pieces,
                      const glm::mat4& ProjectionMatrix,
                      const glm::mat4& ViewMatrix,
                      GLuint MatrixID,
                      GLuint ModelMatrixID,
                      GLuint ViewMatrixID) {
    for (const auto& piece : pieces) {
        glBindBuffer(GL_ARRAY_BUFFER, piece.vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glBindBuffer(GL_ARRAY_BUFFER, piece.uvbuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glBindBuffer(GL_ARRAY_BUFFER, piece.normalbuffer);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, piece.elementbuffer);

        glm::mat4 ModelMatrix = piece.getTransform();
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

        glDrawElements(GL_TRIANGLES, piece.indices.size(), GL_UNSIGNED_SHORT, 0);
    }
}

void movePiece(ChessPiece& piece, int toRow, int toCol) {
    piece.position = ChessboardHelper::getSquarePosition(toRow, toCol);
}