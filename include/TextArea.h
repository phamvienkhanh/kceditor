#ifndef __TEXT_AREA__
#define __TEXT_AREA__
#include <string>
#include <vector>
#include <map>
#include <thread>
#include "ncurses/curses.h"

struct Point
{
    int row;
    int col;
};

struct Size
{
    int width;
    int height;
};

struct Rect
{
    Point pos;
    Size  size;
};


class TextArea
{
private:
    WINDOW* m_window;
    Point   m_cursor;
    Point   m_windPos;
    Size    m_windSize;
    Rect    m_scrollView;

    int mypadpos = 0;
    std::vector<std::string> m_text;
    std::string      m_fileName;
    std::vector<int> m_linesShouldRender;


    int colorComment;
    int colorUserDef;
    std::map<std::string, int> m_colorMap;
    std::map<std::string, int> m_cmUserTypeDef;

    int lineNumberWidth;

    std::thread m_threadParseSyntax;
    bool m_isRunThreadPraseSyntax;

private:
    void moveCursor(int row, int col);
    void appendChar(int row, int col, char ch);
    bool appendCharCurPos(char ch);
    bool deleteCharCurPos();

    void moveCurUp();
    void moveCurDown();
    void moveCurLeft();
    void moveCurRight();

    void breakNewLine();
    void clearRow(int row);
    void clearScreen(int fromRow, int toRow);

    void renderRow(int row);

public:

    void HanldeEvents();
    void Render();
    void DrawBoder();
    void SaveToFile(std::string fileName);
    void OpenFile(std::string fileName);

    void parseUserDefColor();

    TextArea(/* args */);
    ~TextArea();
};

#endif