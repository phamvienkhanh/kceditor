#include "TextArea.h"
#include "utils/lexerUtils.hpp"
#include "utils/json11.hpp"
#include <fstream>
#include <queue>
#include <regex>
#include <chrono>

#define MY_KEY_RETURN 10
#define MY_KEY_BACK 127
#define MY_KEY_TAB 9

extern int g_exitApp;

TextArea::TextArea(/* args */)
{
    lineNumberWidth = 2;
    m_windPos.row = 2;
    m_windPos.col = 4;
    m_windSize.height = LINES - 4;
    m_windSize.width  = COLS - 4 - lineNumberWidth;

    m_cursor.row = 0;
    m_cursor.col = 0;

    m_scrollView.pos  = {0,0};
    m_scrollView.size = {m_windSize.width - 1, m_windSize.height};

    m_window  = newwin(m_windSize.height, m_windSize.width
                        , m_windPos.row, m_windPos.col);
    

    // scrollok(m_window, TRUE);
    keypad(m_window, TRUE);
    wclear(m_window);
    wmove(m_window, 0, 0);
    wrefresh(m_window);
    
    m_text.push_back("");

    // load syntax file
    std::ifstream t("./syntax.json");
    std::string str((std::istreambuf_iterator<char>(t)),
                  std::istreambuf_iterator<char>());
  
    std::string err_comment;
    auto json_comment = json11::Json::parse(str.c_str(), err_comment);
    auto listC = json_comment["configurations"].array_items();

    colorComment = json_comment["comment"].int_value();
    colorUserDef = json_comment["user_def"].int_value();

    int idColor = 1;
    init_color(0, 1000, 0, 0);

    for(auto iItem : listC)
    {
      int colorN  = iItem["fg"].int_value();
      init_pair(idColor, colorN, -1);

      json11::Json::array keys  = iItem["keys"].array_items();
      for(auto& key : keys)
      {
          m_colorMap[key.string_value()]  = idColor;
      }

      idColor++;
    }

    m_threadParseSyntax = std::thread([&](){
        while (m_isRunThreadPraseSyntax)
        {
            this->parseUserDefColor();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    });
}

void TextArea::moveCurUp()
{
    bool isUp = false;
    if(m_cursor.row > 0)
    {
        m_cursor.row--;
        isUp = true;
    }
    else if(m_cursor.row == 0)
    {
        if(m_scrollView.pos.row > 0)
        {
            m_scrollView.pos.row--;
            isUp = true;
        }
    }

    if(isUp)
    {
        int rowIndex = m_scrollView.pos.row + m_cursor.row;
        if(rowIndex >= 0)
        {
            if(m_scrollView.pos.col + m_cursor.col > m_text[rowIndex].size())
            {
                if(m_scrollView.pos.col < m_text[rowIndex].size())
                {
                    m_cursor.col = m_text[rowIndex].size() - m_scrollView.pos.col;
                }
                else
                {
                    if(m_text[rowIndex].size() > m_scrollView.size.width)
                    {
                        m_scrollView.pos.col = m_text[rowIndex].size() - 3;
                        m_cursor.col = 3;
                    }
                    else
                    {
                        m_scrollView.pos.col = 0;
                        m_cursor.col = m_text[rowIndex].size();
                    }
                    
                }
            }
        }
    }
}

void TextArea::moveCurDown()
{
    if(m_cursor.row <  m_text.size() - 1)
    {
        bool isDown = false;
        if(m_cursor.row < m_scrollView.size.height - 1)
        {
            if(m_scrollView.pos.row + m_cursor.row + 1 < m_text.size())
            {
                m_cursor.row++;
                isDown = true;
            }
        }
        else if(m_cursor.row == m_scrollView.size.height - 1)
        {
            if(m_scrollView.pos.row < m_text.size() - m_scrollView.size.height)
            {
                m_scrollView.pos.row++;
                isDown = true;
            }
        }

        if(isDown)
        {
            int rowIndex = m_scrollView.pos.row + m_cursor.row;
            if(rowIndex < m_text.size())
            {
                if(m_scrollView.pos.col + m_cursor.col > m_text[rowIndex].size())
                {
                    if(m_scrollView.pos.col < m_text[rowIndex].size())
                    {
                        m_cursor.col = m_text[rowIndex].size() - m_scrollView.pos.col;
                    }
                    else
                    {
                        if(m_text[rowIndex].size() > m_scrollView.size.width)
                        {
                            m_scrollView.pos.col = m_text[rowIndex].size() - 3;
                            m_cursor.col = 3;
                        }
                        else
                        {
                            m_scrollView.pos.col = 0;
                            m_cursor.col = m_text[rowIndex].size();
                        }
                        
                    }
                }
                // else
                // {
                //     if(m_)
                    
                // }
                
            }
        }
    }
}

void TextArea::moveCurLeft()
{
    if(m_cursor.col > 0)
    {
        m_cursor.col--;
    }
    else if(m_cursor.col == 0)
    {
        if(m_scrollView.pos.col > 0)
        {
            m_scrollView.pos.col--;
        }
    }
}

void TextArea::moveCurRight()
{
    int index = m_cursor.row + m_scrollView.pos.row;
    if(index >= m_text.size())
        return;
        
    if(m_cursor.col < m_text[index].size())
    {
        if(m_cursor.col < m_scrollView.size.width)
        {
            m_cursor.col++;
        }
        else if(m_cursor.col == m_scrollView.size.width)
        {
            if(m_scrollView.pos.col < m_text[index].size() - m_scrollView.size.width)
            {
                m_scrollView.pos.col++;
            }
        }
    }
}

void TextArea::breakNewLine()
{
    int rowIndex = m_cursor.row  + m_scrollView.pos.row;
    if(rowIndex > m_text.size() + 1)
        return;

    m_text.insert(m_text.begin() + rowIndex + 1, "");
    
    std::string newStr = "";

    int colIndex = m_cursor.col + m_scrollView.pos.col;
    newStr = m_text[rowIndex].substr(colIndex);
    m_text[rowIndex].erase(colIndex);

    clearRow(rowIndex);

    m_text[rowIndex + 1] = newStr;

    if(m_cursor.row < m_scrollView.size.height - 1)
    {
        m_cursor.row++;
    }
    else if(m_cursor.row == m_scrollView.size.height - 1)
    {
        m_scrollView.pos.row++;
    }

    m_cursor.col = 0;
    m_scrollView.pos.col = 0;
    for(int i = m_cursor.row - 1; i < m_text.size(); i++)
        m_linesShouldRender.push_back(i);
}

bool TextArea::appendCharCurPos(char c)
{
    if(c < 32 or c > 126)
        return false;

    int maxRow   = m_text.size();
    int rowIndex = m_scrollView.pos.row + m_cursor.row;
    if(rowIndex > maxRow)
        return  false;

    int colIndex = m_cursor.col + m_scrollView.pos.col;
    if(colIndex > m_text[rowIndex].size())
        return false;

    if(m_text[rowIndex].size() == 0)
    {
        m_text[rowIndex].push_back(c);
        return true;
    }
    else
    {
        m_text[rowIndex].insert(colIndex, 1, c);
        return true;
    }  

    return false;
     
}

bool TextArea::deleteCharCurPos()
{
    int maxRow   = m_text.size();
    int rowIndex = m_scrollView.pos.row + m_cursor.row;
    if(rowIndex > maxRow)
        return  false;

    int colIndex = m_cursor.col + m_scrollView.pos.col;
    if(colIndex > m_text[rowIndex].size())
        return false;

    if(colIndex == 0)
    {
        if(rowIndex - 1 >= 0)
        {
            int lenPreLine = m_text[rowIndex - 1].size();
            m_text[rowIndex - 1].append(m_text[rowIndex]);
            m_text.erase( m_text.begin() + rowIndex);
            
            // move cursor up
            if(m_cursor.row > 0)
            {
                m_cursor.row--;
            }
            else if(m_cursor.row == 0)
            {
                m_scrollView.pos.row--;
            }

            if(m_scrollView.pos.col >= lenPreLine
                || lenPreLine > m_scrollView.pos.col + m_scrollView.size.width)
            {
                if(lenPreLine < 3)
                {
                    m_scrollView.pos.col = 0;
                    m_cursor.col = 0;
                }
                else
                {
                    m_scrollView.pos.col = lenPreLine - 3;
                    m_cursor.col = 3;
                }
                
            }
            else
            {
                m_cursor.col = lenPreLine;
            }
        }
    }
    else
    {
        m_text[rowIndex].erase(colIndex - 1, 1);
        moveCurLeft();
    }

    return true;
}

void TextArea::appendChar(int row, int col, char c)
{
    
}

void TextArea::HanldeEvents()
{
    int c = wgetch(m_window);

    switch (c)
    {
    case KEY_UP:
        moveCurUp();
        break;

    case KEY_DOWN:
        moveCurDown();
        break;

    case KEY_LEFT:
        moveCurLeft();
        break;

    case KEY_RIGHT:
        moveCurRight();
        break;

    case MY_KEY_RETURN:
        breakNewLine();
        break;

    case MY_KEY_BACK:
        deleteCharCurPos();
        break;

    case MY_KEY_TAB:
        for(int i = 0; i < 4; i++) {
            if(appendCharCurPos(' '))
            moveCurRight();
        }
        break;

    case KEY_CLOSE:
        g_exitApp = true;
        break;
    
    case KEY_F(2):
        SaveToFile("./testFile.cpp");
        break;

    case KEY_RESIZE:
        
        break;

    case KEY_F(4):
        g_exitApp = true;
        break;

    default:
        if(appendCharCurPos(c))
            moveCurRight();
        break;
    }
}

void TextArea::clearRow(int row)
{
    wmove(m_window, row, 0);
    for(int i=0;i<COLS;i++) 
    {
        waddch(m_window, ' ');
    }
    wmove(m_window, m_windPos.row, m_windPos.col);
}
    
void TextArea::clearScreen(int fromRow, int toRow)
{

}

void TextArea::renderRow(int row)
{
    if(row > m_text.size() || row < 0)
        return;

    wmove(m_window, row, 0);
    wprintw(m_window, m_text[row].c_str());
    wmove(m_window, m_cursor.row, m_cursor.col);
}

void TextArea::DrawBoder()
{
    for(int iRow = 0; iRow < m_windSize.height; iRow++)
    {
        mvaddch(iRow + m_windPos.row, m_windPos.col - lineNumberWidth - 1, ACS_VLINE); // ─ ┌ ┐ ┘ └
        mvaddch(iRow + m_windPos.row, m_windPos.col + m_windSize.width, ACS_VLINE);
    }

    for (int iCol = 0; iCol < m_windSize.width + lineNumberWidth; iCol++)
    {
        mvaddch(m_windPos.row - 1 , iCol + m_windPos.col - lineNumberWidth, ACS_HLINE); // ─ ┌ ┐ ┘ └
        mvaddch(m_windPos.row + m_windSize.height, iCol + m_windPos.col - lineNumberWidth, ACS_HLINE);
    }

    mvaddch(m_windPos.row - 1, m_windPos.col - lineNumberWidth - 1, ACS_ULCORNER);
    mvaddch(m_windPos.row - 1, m_windPos.col + m_windSize.width, ACS_URCORNER);
    mvaddch(m_windPos.row + m_windSize.height, m_windPos.col - lineNumberWidth - 1, ACS_LLCORNER);
    mvaddch(m_windPos.row + m_windSize.height, m_windPos.col + m_windSize.width, ACS_LRCORNER);

    
    wmove(m_window, 0, 0);
    refresh();
}

void TextArea::Render()
{
    // wclear(m_window);
    // renderRow(m_cursor.row);

    // if(!m_linesShouldRender.empty())
    // {
    //     for(auto row : m_linesShouldRender)
    //     {
    //         clearRow(row);
    //         renderRow(row);
    //     }

    //     m_linesShouldRender.clear();
    // }

    for(int row = 0; row < m_scrollView.size.height; row++)
    {
        int rowInText = row + m_scrollView.pos.row;
        if(rowInText >= m_text.size())
            break;
        
        std::string lineTruncate = "";
        int colInText = m_scrollView.pos.col;
        if(colInText < m_text[rowInText].size())
            lineTruncate = m_text[rowInText].substr(colInText, m_scrollView.size.width);
        
        clearRow(row);
        wmove(m_window, row, 0);

        // wprintw(m_window, lineTruncate.c_str());

        // parseUserDefColor();

        Lexer lex(lineTruncate.c_str());
        for (auto token = lex.next();
            not token.is_one_of(Token::Kind::End, Token::Kind::Unexpected);
            token = lex.next()) 
        {
            std::string strToken = token.lexeme();
            int colorId = 0;

            if(token.kind() == Token::Kind::Comment) 
            {
                colorId = colorComment;
            }
            else
            {
                colorId = m_colorMap[strToken];
                if(colorId == 0)
                    colorId = m_cmUserTypeDef[strToken];
            }

            if(colorId != 0)
            {
                wattron(m_window, COLOR_PAIR(colorId));
                wprintw(m_window, strToken.c_str());
                wattroff(m_window, COLOR_PAIR(colorId));
            }
            else
            {
                wprintw(m_window, strToken.c_str());
            }
        }
    }

    wmove(m_window, m_cursor.row, m_cursor.col);
    // box(m_window, 0, 0);
    wrefresh(m_window);
}

void TextArea::parseUserDefColor()
{
    std::map<std::string, int> mapTemp;
    std::smatch typeMatch;
    std::regex  typeRegx(R"(class\s([A-Za-z0-9]+))");
    std::vector<std::string> textClone  = m_text;

    for(auto iLine : textClone)
    {
        if(std::regex_search(iLine, typeMatch, typeRegx)) {
            if (typeMatch.size() > 1) {
                mapTemp[typeMatch[1].str()] = colorUserDef;
            }
        }
    }

    m_cmUserTypeDef.clear();
    m_cmUserTypeDef = mapTemp;
}

void TextArea::SaveToFile(std::string fileName)
{
    std::ofstream fileSave;
    fileSave.open(fileName);
    if(fileSave.is_open())
    {
        for(auto& line : m_text)
        {
            fileSave <<  line  << '\n';
        }

        fileSave.close();
    }
}

TextArea::~TextArea()
{
    m_isRunThreadPraseSyntax = false;
    m_threadParseSyntax.join();
}
