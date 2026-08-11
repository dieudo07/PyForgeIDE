#include "SyntaxHighlighter.hpp"
#include <QFont>
#include <cctype>
#include <algorithm>

namespace PyForge {

const std::unordered_set<std::string> PythonLexer::KEYWORDS = {
    "False","None","True","and","as","assert","async","await",
    "break","class","continue","def","del","elif","else","except",
    "finally","for","from","global","if","import","in","is",
    "lambda","match","case","nonlocal","not","or","pass","raise",
    "return","try","type","while","with","yield"
};

const std::unordered_set<std::string> PythonLexer::BUILTINS = {
    "abs","all","any","bin","bool","bytes","callable","chr","compile",
    "complex","dict","dir","divmod","enumerate","eval","exec","filter",
    "float","format","frozenset","getattr","globals","hasattr","hash",
    "help","hex","id","input","int","isinstance","issubclass","iter",
    "len","list","locals","map","max","memoryview","min","next","object",
    "oct","open","ord","pow","print","property","range","repr","reversed",
    "round","set","setattr","slice","sorted","staticmethod","str","sum",
    "super","tuple","type","vars","zip"
};

const std::unordered_set<std::string> PythonLexer::TYPES = {
    "int","str","float","bool","list","dict","set","tuple","bytes",
    "Optional","Union","List","Dict","Set","Tuple","Any","Callable",
    "Iterator","Generator","ClassVar","Final","Literal","TypeVar",
    "Generic","Protocol","TypedDict","NamedTuple","Self","Never"
};

const std::unordered_set<std::string> PythonLexer::CONSTANTS = {
    "True","False","None","NotImplemented","Ellipsis","__debug__"
};

const std::unordered_set<std::string> PythonLexer::MAGIC_METHODS = {
    "__init__","__new__","__del__","__repr__","__str__","__len__",
    "__getitem__","__setitem__","__delitem__","__iter__","__next__",
    "__call__","__enter__","__exit__","__add__","__sub__","__mul__",
    "__truediv__","__floordiv__","__mod__","__pow__","__eq__","__ne__",
    "__lt__","__le__","__gt__","__ge__","__hash__","__bool__",
    "__contains__","__await__","__aiter__","__anext__"
};

PythonLexer::PythonLexer() {
    updateFormats(Theme::instance().colors);
}

void PythonLexer::updateFormats(const ColorScheme& c) {
    auto f = [](QColor color, bool bold=false, bool italic=false) {
        QTextCharFormat fmt;
        fmt.setForeground(color);
        if (bold)   fmt.setFontWeight(QFont::Bold);
        if (italic) fmt.setFontItalic(true);
        return fmt;
    };
    formats_[TokenType::Keyword]     = f(c.syn_keyword, true);
    formats_[TokenType::Builtin]     = f(c.syn_builtin);
    formats_[TokenType::String]      = f(c.syn_string);
    formats_[TokenType::FString]     = f(c.syn_fstring);
    formats_[TokenType::Number]      = f(c.syn_number);
    formats_[TokenType::Comment]     = f(c.syn_comment, false, true);
    formats_[TokenType::Decorator]   = f(c.syn_decorator);
    formats_[TokenType::FunctionDef] = f(c.syn_function, true);
    formats_[TokenType::ClassDef]    = f(c.syn_class, true);
    formats_[TokenType::Operator]    = f(c.syn_operator);
    formats_[TokenType::Self]        = f(c.syn_self, false, true);
    formats_[TokenType::Type]        = f(c.syn_type);
    formats_[TokenType::Import]      = f(c.syn_import, true);
    formats_[TokenType::Constant]    = f(c.syn_constant, true);
    formats_[TokenType::MagicMethod] = f(c.syn_decorator, false, true);
}

QTextCharFormat PythonLexer::formatFor(TokenType t) const {
    auto it = formats_.find(t);
    return it != formats_.end() ? it->second : QTextCharFormat();
}

std::vector<Token> PythonLexer::tokenize(const QString& text) const {
    std::vector<Token> tokens;
    const int len = text.length();
    int pos = 0;
    bool expectFunc = false, expectClass = false;

    while (pos < len) {
        const QChar ch = text[pos];
        const char cc  = ch.toLatin1();

        if (ch.isSpace()) { ++pos; continue; }

        if (cc == '#') { pos = parseComment(text, pos, tokens); continue; }

        if (cc == '@') {
            int start = pos++;
            while (pos < len && (text[pos].isLetterOrNumber() || text[pos] == '_' || text[pos] == '.'))
                ++pos;
            tokens.push_back({start, pos-start, TokenType::Decorator});
            continue;
        }

        if (cc == '"' || cc == '\'' ||
            ((cc=='f'||cc=='F'||cc=='r'||cc=='R'||cc=='b'||cc=='B') &&
             pos+1<len && (text[pos+1]=='"'||text[pos+1]=='\''))) {
            pos = parseString(text, pos, tokens);
            continue;
        }

        if (std::isdigit(cc) || (cc=='.' && pos+1<len && std::isdigit(text[pos+1].toLatin1()))) {
            pos = parseNumber(text, pos, tokens);
            continue;
        }

        if (std::isalpha(cc) || cc == '_') {
            pos = parseIdentifier(text, pos, tokens);
            // Vérifier les mots-clés def/class
            const auto& last = tokens.back();
            const std::string word = text.mid(last.start, last.length).toStdString();
            if (last.type == TokenType::Keyword) {
                if (word == "def")   expectFunc  = true;
                if (word == "class") expectClass = true;
            } else if (expectFunc && last.type == TokenType::None) {
                tokens.back().type = TokenType::FunctionDef;
                expectFunc = false;
            } else if (expectClass && last.type == TokenType::None) {
                tokens.back().type = TokenType::ClassDef;
                expectClass = false;
            }
            continue;
        }

        static const std::string ops = "+-*/%&|^~<>=!";
        if (ops.find(cc) != std::string::npos) {
            // Opérateurs 2 caractères
            if (pos+1 < len) {
                char nc = text[pos+1].toLatin1();
                if ((cc=='-'&&nc=='>')||(cc=='='&&nc=='=')||
                    (cc=='!'&&nc=='=')||(cc=='<'&&nc=='=')||
                    (cc=='>'&&nc=='=')||(cc=='*'&&nc=='*')||
                    (cc=='/'&&nc=='/')||(cc=='+'&&nc=='=')||
                    (cc=='-'&&nc=='=')||(cc=='*'&&nc=='=')||
                    (cc=='/'&&nc=='=')) {
                    tokens.push_back({pos, 2, TokenType::Operator});
                    pos += 2; continue;
                }
            }
            tokens.push_back({pos, 1, TokenType::Operator});
            ++pos; continue;
        }

        ++pos;
    }
    return tokens;
}

int PythonLexer::parseIdentifier(const QString& text, int pos,
                                  std::vector<Token>& tokens) const {
    const int start = pos;
    const int len   = text.length();
    while (pos < len && (text[pos].isLetterOrNumber() || text[pos] == '_'))
        ++pos;
    const std::string word = text.mid(start, pos-start).toStdString();
    TokenType tt = TokenType::None;
    if      (KEYWORDS.count(word))     tt = TokenType::Keyword;
    else if (word=="self"||word=="cls") tt = TokenType::Self;
    else if (CONSTANTS.count(word))    tt = TokenType::Constant;
    else if (BUILTINS.count(word))     tt = TokenType::Builtin;
    else if (TYPES.count(word))        tt = TokenType::Type;
    else if (MAGIC_METHODS.count(word))tt = TokenType::MagicMethod;
    else if (word=="import"||word=="from") tt = TokenType::Import;
    tokens.push_back({start, pos-start, tt});
    return pos;
}

int PythonLexer::parseString(const QString& text, int pos,
                              std::vector<Token>& tokens) const {
    const int start = pos;
    const int len   = text.length();
    TokenType type  = TokenType::String;
    char c = text[pos].toLatin1();
    if (c=='f'||c=='F') { type=TokenType::FString; ++pos; }
    else if (c=='r'||c=='R'||c=='b'||c=='B') { ++pos; }
    if (pos >= len) { tokens.push_back({start,pos-start,type}); return pos; }
    char q = text[pos].toLatin1();
    if (pos+2<len && text[pos+1]==q && text[pos+2]==q) {
        pos += 3;
        while (pos+2 < len) {
            if (text[pos]==q && text[pos+1]==q && text[pos+2]==q) { pos+=3; break; }
            if (text[pos]=='\\') { ++pos; } ++pos;
        }
    } else {
        ++pos;
        while (pos<len && text[pos]!=q && text[pos]!='\n') {
            if (text[pos]=='\\') ++pos; ++pos;
        }
        if (pos<len && text[pos]==q) ++pos;
    }
    tokens.push_back({start, pos-start, type});
    return pos;
}

int PythonLexer::parseNumber(const QString& text, int pos,
                              std::vector<Token>& tokens) const {
    const int start = pos;
    const int len   = text.length();
    char c = text[pos].toLatin1();
    if (c=='0' && pos+1<len) {
        char n=text[pos+1].toLatin1();
        if (n=='x'||n=='X'||n=='o'||n=='O'||n=='b'||n=='B') {
            pos += 2;
            while (pos<len && (std::isxdigit(text[pos].toLatin1())||text[pos]=='_')) ++pos;
            tokens.push_back({start,pos-start,TokenType::Number}); return pos;
        }
    }
    while (pos<len && (std::isdigit(text[pos].toLatin1())||text[pos]=='_')) ++pos;
    if (pos<len && text[pos]=='.') {
        ++pos;
        while (pos<len && (std::isdigit(text[pos].toLatin1())||text[pos]=='_')) ++pos;
    }
    if (pos<len && (text[pos]=='e'||text[pos]=='E')) {
        ++pos;
        if (pos<len && (text[pos]=='+'||text[pos]=='-')) ++pos;
        while (pos<len && std::isdigit(text[pos].toLatin1())) ++pos;
    }
    if (pos<len && (text[pos]=='j'||text[pos]=='J')) ++pos;
    tokens.push_back({start, pos-start, TokenType::Number});
    return pos;
}

int PythonLexer::parseComment(const QString& text, int pos,
                               std::vector<Token>& tokens) const {
    const int start = pos;
    while (pos<text.length() && text[pos]!='\n') ++pos;
    tokens.push_back({start, pos-start, TokenType::Comment});
    return pos;
}

// ── SyntaxHighlighter ────────────────────────────────────────────

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* doc)
    : QSyntaxHighlighter(doc) {}

void SyntaxHighlighter::setTheme(const ColorScheme& c) {
    lexer_.updateFormats(c);
    rehighlight();
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
    const int prev = previousBlockState();
    setCurrentBlockState(Normal);

    // Continuer une triple-quote multi-ligne
    if (prev == TripleDouble || prev == TripleSingle) {
        char q = (prev == TripleDouble) ? '"' : '\'';
        int i = 0;
        while (i < text.length()) {
            if (i+2 < text.length() && text[i]==q && text[i+1]==q && text[i+2]==q) {
                i += 3;
                setFormat(0, i, lexer_.formatFor(TokenType::String));
                setCurrentBlockState(Normal);
                goto lex_rest;
            }
            ++i;
        }
        setFormat(0, text.length(), lexer_.formatFor(TokenType::String));
        setCurrentBlockState(prev);
        return;
    }

lex_rest:
    // Détecter nouvelle triple-quote
    for (int i = 0; i < text.length()-2; ++i) {
        char c = text[i].toLatin1();
        if ((c=='"'||c=='\'') && text[i+1]==c && text[i+2]==c) {
            int j = i+3;
            bool closed = false;
            while (j < text.length()-2) {
                if (text[j]==c && text[j+1]==c && text[j+2]==c) { closed=true; break; }
                ++j;
            }
            if (!closed)
                setCurrentBlockState(c=='"' ? TripleDouble : TripleSingle);
        }
    }

    const auto tokens = lexer_.tokenize(text);
    for (const auto& tok : tokens)
        if (tok.type != TokenType::None)
            setFormat(tok.start, tok.length, lexer_.formatFor(tok.type));
}

} // namespace PyForge
#include "SyntaxHighlighter.moc"