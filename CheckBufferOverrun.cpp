//---------------------------------------------------------------------------
#include "CheckBufferOverrun.h"
#include "Tokenize.h"
#include "CommonCheck.h"

#include <sstream>

#include <stdlib.h>     // <- strtoul

extern bool IsNumber(const char str[]);

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Buffer overrun..
//---------------------------------------------------------------------------

void CheckBufferOverrun()
{
    int indentlevel = 0;
    for (TOKEN *tok = tokens; tok; tok = tok->next)
    {
        if (tok->str[0]=='{')
            indentlevel++;

        else if (tok->str[0]=='}')
            indentlevel--;

        else if (indentlevel > 0)
        {
            // Declaring array..
            if (match(tok, "type var [ num ] ;"))
            {
                const char *varname = getstr(tok,1);
                unsigned int size = strtoul(getstr(tok,3), NULL, 10);
                int _indentlevel = indentlevel;
                for (TOKEN *tok2 = gettok(tok,5); tok2; tok2 = tok2->next)
                {
                    if (tok2->str[0]=='{')
                    {
                        _indentlevel++;
                    }
                    else if (tok2->str[0]=='}')
                    {
                        _indentlevel--;
                        if (_indentlevel < indentlevel)
                            break;
                    }
                    else
                    {
                        // Array index..
                        if (strcmp(tok2->str,varname)==0 &&
                            strcmp(getstr(tok2,1),"[")==0 &&
                            IsNumber(getstr(tok2,2)) &&
                            strcmp(getstr(tok2,3),"]")==0 )
                        {
                            const char *str = getstr(tok2, 2);
                            if (strtoul(str, NULL, 10) >= size)
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(tok2) << ": Array index out of bounds";
                                ReportErr(ostr.str());
                            }
                        }

                        // Loop..
                        const char *strindex = 0;
                        int value = 0;
                        if (match(tok2,"for ( var = 0 ; var < num ; var + + )"))
                        {
                            strindex = getstr(tok2,2);
                            value = atoi(getstr(tok2,8));
                        }
                        else if (match(tok2,"for ( var = 0 ; var <= num ; var + + )"))
                        {
                            strindex = getstr(tok2,2);
                            value = 1 + atoi(getstr(tok2,8));
                        }
                        else if (match(tok2,"for ( var = 0 ; var < num ; + + var )"))
                        {
                            strindex = getstr(tok2,2);
                            value = atoi(getstr(tok2,8));
                        }
                        else if (match(tok2,"for ( var = 0 ; var <= num ; + + var )"))
                        {
                            strindex = getstr(tok2,2);
                            value = 1 + atoi(getstr(tok2,8));
                        }
                        if (strindex && value>(int)size)
                        {
                            TOKEN *tok3 = tok2;
                            while (tok3 && strcmp(tok3->str,")"))
                                tok3 = tok3->next;
                            if (!tok3)
                                break;
                            tok3 = tok3->next;
                            if (tok3->str[0] == '{')
                                tok3 = tok3->next;
                            while (tok3 && !strchr(";}",tok3->str[0]))
                            {
                                if (strcmp(tok3->str,varname)==0 &&
                                    strcmp(getstr(tok3,1),"[")==0 &&
                                    strcmp(getstr(tok3,2),strindex)==0 &&
                                    strcmp(getstr(tok3,3),"]")==0 )
                                {
                                    std::ostringstream ostr;
                                    ostr << FileLine(tok3) << ": Buffer overrun";
                                    ReportErr(ostr.str());
                                    break;
                                }
                                tok3 = tok3->next;
                            }
                        }


                        // Writing data into array..
                        if (match(tok2,"strcpy ( var , "))
                        {
                            int len = 0;
                            if (strcmp(getstr(tok2, 2), varname) == 0)
                            {
                                const char *str = getstr(tok2, 4);
                                if (str[0] == '\"')
                                {
                                    while (*str)
                                    {
                                        if (*str=='\\')
                                            str++;
                                        str++;
                                        len++;
                                    }
                                }
                            }
                            if (len > 2 && len >= (int)size + 2)
                            {
                                std::ostringstream ostr;
                                ostr << FileLine(tok2) << ": Buffer overrun";
                                ReportErr(ostr.str());
                            }
                        }
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------








