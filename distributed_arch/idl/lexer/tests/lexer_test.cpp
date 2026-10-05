#include <gtest/gtest.h>

#include <lexer.h>
#include <token.h>
#include <token_type.h>

using namespace jk_ridl;

TEST(LexerTest, EmptySourceReturnsEOF) {

  Lexer lexer("", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::EndOfFile);
  EXPECT_TRUE(token.lexeme.empty());
}

TEST(LexerTest, LBraceToken) {

  Lexer lexer("{", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::LBrace);
  EXPECT_EQ(token.lexeme, "{");
}

TEST(LexerTest, MessageKeyword) {

  Lexer lexer("message", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::KwMessage);
  EXPECT_EQ(token.lexeme, "message");
}

TEST(LexerTest, IntegerLiteral) {

  Lexer lexer("123", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::IntegerLiteral);
  EXPECT_EQ(token.lexeme, "123");
}

TEST(LexerTest, StringLiteral) {

  Lexer lexer("\"hello\"", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::StringLiteral);
  EXPECT_EQ(token.lexeme, "hello");
}

// keyword recognition test
TEST(LexerTest, Keywords) {

  Lexer lexer("namespace message array vector string u32 f64", "test.ridl");

  EXPECT_EQ(lexer.NextToken().type, TokenType::KwNamespace);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Whitespace);

  EXPECT_EQ(lexer.NextToken().type, TokenType::KwMessage);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Whitespace);

  EXPECT_EQ(lexer.NextToken().type, TokenType::KwArray);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Whitespace);

  EXPECT_EQ(lexer.NextToken().type, TokenType::KwVector);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Whitespace);

  EXPECT_EQ(lexer.NextToken().type, TokenType::KwString);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Whitespace);

  EXPECT_EQ(lexer.NextToken().type, TokenType::KwU32);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Whitespace);

  EXPECT_EQ(lexer.NextToken().type, TokenType::KwF64);
}

// identifier recognition test
TEST(LexerTest, Identifiers) {

  Lexer lexer("MotorState rpm wheel_speed", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.lexeme, "MotorState");

  token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.lexeme, "rpm");

  token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.lexeme, "wheel_speed");
}

// integer literal recognition test
TEST(LexerTest, IntegerLiterals) {

  Lexer lexer("0 1 42 65535", "test.ridl");

  EXPECT_EQ(lexer.NextToken().lexeme, "0");
  (void)lexer.NextToken();

  EXPECT_EQ(lexer.NextToken().lexeme, "1");
  (void)lexer.NextToken();

  EXPECT_EQ(lexer.NextToken().lexeme, "42");
  (void)lexer.NextToken();

  EXPECT_EQ(lexer.NextToken().lexeme, "65535");
}

// string literal recognition test
TEST(LexerTest, StringLiterals) {

  Lexer lexer("\"hello\" \"robotics\"", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::StringLiteral);
  EXPECT_EQ(token.lexeme, "hello");

  (void)lexer.NextToken();

  token = lexer.NextToken();

  EXPECT_EQ(token.type, TokenType::StringLiteral);
  EXPECT_EQ(token.lexeme, "robotics");
}

// punctuation token recognition test
TEST(LexerTest, PunctuationTokens) {

  Lexer lexer("{}<>;,.", "test.ridl");

  EXPECT_EQ(lexer.NextToken().type, TokenType::LBrace);
  EXPECT_EQ(lexer.NextToken().type, TokenType::RBrace);
  EXPECT_EQ(lexer.NextToken().type, TokenType::LAngle);
  EXPECT_EQ(lexer.NextToken().type, TokenType::RAngle);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Semicolon);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Comma);
  EXPECT_EQ(lexer.NextToken().type, TokenType::Dot);
}

// comprehensive test for a message declaration
TEST(LexerTest, MessageDeclaration) {

  const std::string source = R"(
namespace robotics {

message MotorState {
    u32 rpm;
    string<64> name;
}

}
)";

  Lexer lexer(source, "test.ridl");

  Token token;

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::KwNamespace);
  EXPECT_EQ(token.lexeme, "namespace");

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.lexeme, "robotics");

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::LBrace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::KwMessage);
  EXPECT_EQ(token.lexeme, "message");

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.lexeme, "MotorState");

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::LBrace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::KwU32);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.lexeme, "rpm");

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Semicolon);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::KwString);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::LAngle);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::IntegerLiteral);
  EXPECT_EQ(token.lexeme, "64");

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::RAngle);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Whitespace);

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Identifier);
  EXPECT_EQ(token.lexeme, "name");

  token = lexer.NextToken();
  EXPECT_EQ(token.type, TokenType::Semicolon);
}

// test for correct source location tracking
TEST(LexerTest, SourceLocation) {

  Lexer lexer("message\nMotorState", "test.ridl");

  Token token = lexer.NextToken();

  EXPECT_EQ(token.range.start.line, 1);
  EXPECT_EQ(token.range.start.column, 1);

  (void)lexer.NextToken(); // whitespace

  token = lexer.NextToken();

  EXPECT_EQ(token.lexeme, "MotorState");
  EXPECT_EQ(token.range.start.line, 2);
  EXPECT_EQ(token.range.start.column, 1);
}
