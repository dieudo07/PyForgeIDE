#pragma once
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "../utils/Theme.hpp"

namespace PyForge {

enum class TokenType {
    None, Keyword, Builtin, String, FString,
    Number, Comment, Decorator, FunctionDef,
    ClassDef, Operator, Self, Type, Import,
    Constant, MagicMethod
};

struct Token { int start, length; TokenType type; };

class PythonLexer {
public:
    PythonLexer();
    std::vector<Token> tokenize(const QString& text) const;
    void updateFormats(const ColorScheme& colors);
    QTextCharFormat formatFor(TokenType type) const;

private:
    static const std::unordered_set<std::string> KEYWORDS;
    static const std::unordered_set<std::string> BUILTINS;
    static const std::unordered_set<std::string> TYPES;
    static const std::unordered_set<std::string> CONSTANTS;
    static const std::unordered_set<std::string> MAGIC_METHODS;

    std::unordered_map<TokenType, QTextCharFormat> formats_;

    int parseString(const QString& t, int pos, std::vector<Token>& toks) const;
    int parseNumber(const QString& t, int pos, std::vector<Token>& toks) const;
    int parseComment(const QString& t, int pos, std::vector<Token>& toks) const;
    int parseIdentifier(const QString& t, int pos, std::vector<Token>& toks) const;
};

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument* doc);
    void setTheme(const ColorScheme& colors);

protected:
    void highlightBlock(const QString& text) override;

private:
    PythonLexer lexer_;
    enum State { Normal=0, TripleDouble=1, TripleSingle=2 };
};

} // namespace PyForge