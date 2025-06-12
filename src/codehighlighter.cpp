#include "../include/codehighlighter.h"

CodeHighlighter::CodeHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    setupThemeFormats("light");
    setupHighlightingRules();
}

void CodeHighlighter::setupHighlightingRules()
{
    highlightingRules.clear();

    // 关键字
    QStringList keywordPatterns = {
        "\\bclass\\b", "\\bconst\\b", "\\benum\\b", "\\bexplicit\\b",
        "\\bfriend\\b", "\\binline\\b", "\\bnamespace\\b", "\\boperator\\b",
        "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b", "\\bsignals\\b",
        "\\bsigned\\b", "\\bslots\\b", "\\bstatic\\b", "\\bstruct\\b",
        "\\btemplate\\b", "\\btypedef\\b", "\\btypename\\b", "\\bunion\\b",
        "\\bunsigned\\b", "\\bvirtual\\b", "\\bvolatile\\b", "\\bbool\\b",
        "\\bbreak\\b", "\\bcase\\b", "\\bcatch\\b", "\\bchar\\b",
        "\\bcontinue\\b", "\\bdefault\\b", "\\bdelete\\b", "\\bdo\\b",
        "\\bdouble\\b", "\\belse\\b", "\\bfloat\\b", "\\bfor\\b",
        "\\bgoto\\b", "\\bif\\b", "\\bint\\b", "\\blong\\b",
        "\\bnew\\b", "\\breturn\\b", "\\bshort\\b", "\\bswitch\\b",
        "\\bthrow\\b", "\\btry\\b", "\\bvoid\\b", "\\bwhile\\b",
        // Python关键字
        "\\bdef\\b", "\\bpass\\b", "\\braise\\b", "\\bfrom\\b",
        "\\bimport\\b", "\\bas\\b", "\\bglobal\\b", "\\bassert\\b",
        "\\belse\\b", "\\bif\\b", "\\bpass\\b", "\\bbreak\\b",
        "\\bexcept\\b", "\\bin\\b", "\\braise\\b", "\\breturn\\b",
        // Java关键字
        "\\babstract\\b", "\\bextends\\b", "\\bimplements\\b", "\\bthrows\\b",
        "\\bsynchronized\\b", "\\bvolatile\\b", "\\btransient\\b"
    };

    for (const QString &pattern : keywordPatterns) {
        HighlightingRule rule;
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // 类名
    HighlightingRule rule;
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // 单行注释
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Python单行注释
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // 字符串
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // 函数
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // 数字
    rule.pattern = QRegularExpression("\\b\\d+\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 多行注释
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void CodeHighlighter::setupThemeFormats(const QString &theme)
{
    if (theme == "dark") {
        keywordFormat.setForeground(Qt::cyan);
        classFormat.setForeground(QColor("#4EC9B0"));
        singleLineCommentFormat.setForeground(QColor("#6A9955"));
        multiLineCommentFormat.setForeground(QColor("#6A9955"));
        quotationFormat.setForeground(QColor("#CE9178"));
        functionFormat.setForeground(QColor("#DCDCAA"));
        numberFormat.setForeground(QColor("#B5CEA8"));
    } else if (theme == "light") {
        keywordFormat.setForeground(QColor("#0000FF"));
        classFormat.setForeground(QColor("#267F99"));
        singleLineCommentFormat.setForeground(QColor("#008000"));
        multiLineCommentFormat.setForeground(QColor("#008000"));
        quotationFormat.setForeground(QColor("#A31515"));
        functionFormat.setForeground(QColor("#795E26"));
        numberFormat.setForeground(QColor("#098658"));
    }

    keywordFormat.setFontWeight(QFont::Bold);
    classFormat.setFontWeight(QFont::Bold);
}

void CodeHighlighter::setTheme(const QString &theme)
{
    setupThemeFormats(theme);
    setupHighlightingRules();
    rehighlight();
}

void CodeHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // 多行注释的处理
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
} 