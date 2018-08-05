/*
 *    draw.c
 *
 *    Routines for drawing text on images
 *
 *    Copyright 2000, Jeroen Vreeken
 *    This program is published under the GNU public license version 2
 *    See also the file 'COPYING'
 *
 */

#include <ctype.h>
#include "motion.h"

/* Highest ascii value is 126 (~) */
#define ASCII_MAX 127

unsigned char *char_arr_ptr[ASCII_MAX];

struct draw_char {
    unsigned char ascii;
    unsigned char pix[8][7];
};

struct draw_char draw_table[]= {
    {
        ' ',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        '0',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,2,2,1},
            {1,2,1,2,1,2,1},
            {1,2,1,2,1,2,1},
            {1,2,2,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        '1',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        '2',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {0,1,1,2,2,1,0},
            {0,1,2,1,1,0,0},
            {1,2,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,1,1,1,0}
        }
    },
    {
        '3',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {0,1,1,2,2,1,0},
            {0,1,0,1,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        '4',
        {
            {0,0,0,0,1,0,0},
            {0,0,0,1,2,1,0},
            {0,0,1,2,2,1,0},
            {0,1,2,1,2,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,1,2,1,0},
            {0,0,0,1,2,1,0},
            {0,0,0,0,1,0,0}
        }
    },
    {
        '5',
        {
            {0,1,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {1,2,1,1,1,1,0},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,2,0},
            {0,1,1,1,1,2,0},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        '6',
        {
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {1,2,1,1,1,1,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        '7',
        {
            {0,1,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,1,1,2,1},
            {0,0,0,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,1,2,1,0,0,0},
            {0,1,2,1,0,0,0},
            {0,0,1,0,0,0,0}
        }
    },
    {
        '8',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        '9',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,2,1},
            {0,1,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        '"',
        {
            {0,0,1,0,1,0,0},
            {0,1,2,1,2,1,0},
            {0,1,2,1,2,1,0},
            {0,0,1,0,1,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        '/',
        {
            {0,0,0,0,1,0,0},
            {0,0,0,1,2,1,0},
            {0,0,0,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,1,0,0,0},
            {0,1,2,1,0,0,0},
            {0,0,1,0,0,0,0}
        }
    },
    {
        '(',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,1,0,0,0},
            {0,1,2,1,0,0,0},
            {0,1,2,1,0,0,0},
            {0,1,2,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        ')',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,2,1,0},
            {0,0,0,1,2,1,0},
            {0,0,0,1,2,1,0},
            {0,0,0,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        '@',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,2,2,2,1},
            {1,2,1,2,2,2,1},
            {1,2,1,1,1,1,0},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        '~',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,0,0,0,0},
            {0,1,2,1,0,1,0},
            {1,2,1,2,1,2,1},
            {0,1,0,1,2,1,0},
            {0,0,0,0,1,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        '#',
        {
            {0,0,1,0,1,0,0},
            {0,1,2,1,2,1,0},
            {1,2,2,2,2,2,1},
            {0,1,2,1,2,1,0},
            {0,1,2,1,2,1,0},
            {1,2,2,2,2,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,0,1,0,0}
        }
    },
    {
        '<',
        {
            {0,0,0,0,0,1,0},
            {0,0,0,1,1,2,1},
            {0,1,1,2,2,1,0},
            {1,2,2,1,1,0,0},
            {0,1,1,2,2,1,0},
            {0,0,0,1,1,2,1},
            {0,0,0,0,0,1,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        '>',
        {
            {0,1,0,0,0,0,0},
            {1,2,1,1,0,0,0},
            {0,1,2,2,1,1,0},
            {0,0,1,1,2,2,1},
            {0,1,2,2,1,1,0},
            {1,2,1,1,0,0,0},
            {0,1,0,0,0,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        '|',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        ',',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,0,0,0},
            {0,1,2,2,1,0,0},
            {0,1,2,2,1,0,0},
            {0,1,2,1,0,0,0},
            {0,0,1,0,0,0,0}
        }
    },
    {
        '.',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,0,0,0},
            {0,1,2,2,1,0,0},
            {0,1,2,2,1,0,0},
            {0,0,1,1,0,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        ':',
        {
            {0,0,1,1,0,0,0},
            {0,1,2,2,1,0,0},
            {0,1,2,2,1,0,0},
            {0,0,1,1,0,0,0},
            {0,0,1,1,0,0,0},
            {0,1,2,2,1,0,0},
            {0,1,2,2,1,0,0},
            {0,0,1,1,0,0,0}
        }
    },
    {
        '-',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        '+',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,2,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        '_',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,1,1,1,0}
        }
    },
    {
        '\'',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0}
        }
    },
    {
        'a',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,2,1},
            {0,0,1,1,1,1,0}
        }
    },
    {
        'b',
        {
            {0,1,0,0,0,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        'c',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {1,2,1,1,1,1,0},
            {1,2,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {0,0,1,1,1,1,0}
        }
    },
    {
        'd',
        {
            {0,0,0,0,0,1,0},
            {0,0,0,0,1,2,1},
            {0,0,1,1,1,2,1},
            {0,1,2,2,2,2,1},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,2,1},
            {0,0,1,1,1,1,0}
        }
    },
    {
        'e',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,2,1,1,2,1},
            {1,2,1,2,2,1,0},
            {0,1,2,2,2,2,1},
            {0,0,1,1,1,1,0}
        }
    },
    {
        'f',
        {
            {0,0,0,0,1,1,0},
            {0,0,0,1,2,2,1},
            {0,0,1,2,1,1,0},
            {0,1,2,2,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        'g',
        {
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,2,1},
            {0,1,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        'h',
        {
            {0,1,0,0,0,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,1,1,0,0},
            {1,2,1,2,2,1,0},
            {1,2,2,1,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'i',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        'j',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,1,1,2,1,0,0},
            {1,2,2,1,0,0,0},
            {0,1,1,0,0,0,0}
        }
    },
    {
        'k',
        {
            {0,1,0,0,0,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,0,1,0,0},
            {1,2,1,1,2,1,0},
            {1,2,1,2,1,0,0},
            {1,2,2,1,2,1,0},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'l',
        {
            {0,0,1,1,0,0,0},
            {0,1,2,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,2,1,0},
            {0,0,0,0,1,0,0}
        }
    },
    {
        'm',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,1,0,1,0,0},
            {1,2,2,1,2,1,0},
            {1,2,1,2,1,2,1},
            {1,2,1,2,1,2,1},
            {1,2,1,2,1,2,1},
            {0,1,0,1,0,1,0}
        }
    },
    {
        'n',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,0,1,1,0,0},
            {1,2,1,2,2,1,0},
            {1,2,2,1,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'o',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        'p',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,0,0},
            {1,2,1,0,0,0,0},
        }
    },
    {
        'q',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,2,1},
            {0,0,1,1,1,2,1},
            {0,0,0,0,1,2,1}
        }
    },
    {
        'r',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,0,1,1,0,0},
            {1,2,1,2,2,1,0},
            {1,2,2,1,1,2,1},
            {1,2,1,0,0,1,0},
            {1,2,1,0,0,0,0},
            {0,1,0,0,0,0,0}
        }
    },
    {
        's',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {1,2,2,2,1,1,0},
            {0,1,1,2,2,2,1},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        't',
        {
            {0,0,0,1,0,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,2,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,2,1,0},
            {0,0,0,0,1,0,0}
        }
    },
    {
        'u',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,1,2,2,1},
            {0,1,2,2,1,2,1},
            {0,0,1,1,0,1,0}
        }
    },
    {
        'v',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        'w',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,1,2,1,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,0,1,0,0}
        }
    },
    {
        'x',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,0,0,1,0,0},
            {1,2,1,1,2,1,0},
            {0,1,2,2,1,0,0},
            {0,1,2,2,1,0,0},
            {1,2,1,1,2,1,0},
            {0,1,0,0,1,0,0}
        }
    },
    {
        'y',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,1,2,1,0,0,0},
            {1,2,1,0,0,0,0}
        }
    },
    {
        'z',
        {
            {0,0,0,0,0,0,0},
            {0,0,0,0,0,0,0},
            {0,1,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {0,1,1,2,1,0,0},
            {0,1,2,1,1,0,0},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        'A',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,2,1},
            {1,2,1,1,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'B',
        {
            {0,1,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        'C',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,0,0,1,0},
            {1,2,1,0,0,1,0},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        'D',
        {
            {0,1,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        'E',
        {
            {0,1,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {1,2,1,1,1,1,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,0,0},
            {1,2,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,1,1,1,0}
        }
    },
    {
        'F',
        {
            {0,1,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {1,2,1,1,1,1,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,0,0,0,0},
            {0,1,0,0,0,0,0}
        }
    },
    {
        'G',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,1,0},
            {1,2,1,2,2,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        'H',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,2,1},
            {1,2,1,1,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'I',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        'J',
        {
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {0,0,1,1,1,2,1},
            {0,0,0,0,1,2,1},
            {0,1,0,0,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        'K',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,1,2,1,0},
            {1,2,1,2,1,0,0},
            {1,2,2,2,1,0,0},
            {1,2,1,1,2,1,0},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'L',
        {
            {0,1,0,0,0,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        'M',
        {
            {0,1,1,0,1,1,0},
            {1,2,2,1,2,2,1},
            {1,2,1,2,1,2,1},
            {1,2,1,1,1,2,},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'N',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,2,1,1,2,1},
            {1,2,1,2,1,2,1},
            {1,2,1,1,2,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'O',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,0,0}
        }
    },
    {
        'P',
        {
            {0,1,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,0,0},
            {1,2,1,0,0,0,0},
            {1,2,1,0,0,0,0},
            {0,1,0,0,0,0,0}
        }
    },
    {
        'Q',
        {
            {0,0,1,1,1,0,0},
            {0,1,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,1,2,1,2,1},
            {1,2,1,1,2,1,0},
            {0,1,2,2,1,2,1},
            {0,0,1,1,0,1,0}
        }
    },
    {
        'R',
        {
            {0,1,1,1,1,0,0},
            {1,2,2,2,2,1,0},
            {1,2,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {1,2,1,2,1,0,0},
            {1,2,1,1,2,1,0},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'S',
        {
            {0,0,1,1,1,1,0},
            {0,1,2,2,2,2,1},
            {1,2,1,1,1,1,0},
            {0,1,2,2,2,1,0},
            {0,0,1,1,1,2,1},
            {0,1,1,1,1,2,1},
            {1,2,2,2,2,1,0},
            {0,1,1,1,1,0,0}
        }
    },
    {
        'T',
        {
            {0,1,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,2,1,1,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        'U',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,1,1,2,1},
            {0,1,2,2,2,2,1},
            {0,0,1,1,1,1,0}
        }
    },
    {
        'V',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        'W',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {1,2,1,0,1,2,1},
            {1,2,1,1,1,2,1},
            {1,2,1,2,1,2,1},
            {1,2,1,2,1,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,0,1,0,0}
        }
    },
    {
        'X',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,1,2,1,2,1,0},
            {1,2,1,0,1,2,1},
            {0,1,0,0,0,1,0}
        }
    },
    {
        'Y',
        {
            {0,1,0,0,0,1,0},
            {1,2,1,0,1,2,1},
            {0,1,2,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,1,2,1,0,0},
            {0,0,0,1,0,0,0}
        }
    },
    {
        'Z',
        {
            {0,1,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,1,2,1,0},
            {0,0,1,2,1,0,0},
            {0,1,2,1,0,0,0},
            {1,2,1,1,1,1,0},
            {1,2,2,2,2,2,1},
            {0,1,1,1,1,1,0}
        }
    }
};

#define NEWLINE "\\n"
/**
 * draw_textn
 */
static int draw_textn(unsigned char *image, int startx,  int starty,  int width, const char *text, int len, int factor)
{

    int x, y;
    int pos, line_offset, next_char_offs;
    unsigned char *image_ptr, *char_ptr;

    if (startx > width / 2)
        startx -= len * (6 * factor);

    if (startx + len * 6 * factor >= width)
        len = (width-startx-1)/(6*factor);

    if ((startx < 1) || (starty < 1) || (len < 1)) return 0;

    line_offset = width - (7 * factor);
    next_char_offs = (width * 8 * factor) - (6 * factor);

    image_ptr = image + startx + (starty * width);

    for (pos = 0; pos < len; pos++) {
        int pos_check = (int)text[pos];

        char_ptr = char_arr_ptr[pos_check];

        for (y = 0; y < 8 * factor; y++) {
            for (x = 0; x < 7 * factor; x++) {

                if (pos_check < 0) {
                    image_ptr++;
                    continue;
                }

                char_ptr = char_arr_ptr[pos_check] + y/factor*7 + x/factor;

                switch(*char_ptr) {
                case 1:
                    *image_ptr = 0;
                     break;
                case 2:
                    *image_ptr = 255;
                    break;
                default:
                    break;
                }

                image_ptr++;
            }
            image_ptr += line_offset;
        }
        image_ptr -= next_char_offs;
    }

    return 0;
}

/**
 * draw_text
 */
int draw_text(unsigned char *image, int width, int height, int startx, int starty, const char *text, int factor)
{
    int num_nl = 0;
    const char *end, *begin;
    int line_space, txtlen;

    /* Count the number of newlines in "text" so we scroll it up the image. */
    begin = end = text;
    txtlen = 0;
    while ((end = strstr(end, NEWLINE))) {
        if ((end - begin)>txtlen) txtlen = (end - begin);
        num_nl++;
        end += sizeof(NEWLINE)-1;
    }
    if (txtlen == 0) txtlen = strlen(text);

    /* Adjust the factor if it is out of bounds
     * txtlen at this point is the approx length of longest line
    */
    if ((txtlen * 7 * factor) > width){
        factor = (width / (txtlen * 7));
        if (factor <= 0) factor = 1;
    }

    if (((num_nl+1) * 8 * factor) > height){
        factor = (height / ((num_nl+1) * 8));
        if (factor <= 0) factor = 1;
    }

    line_space = factor * 9;

    starty -= line_space * num_nl;

    begin = end = text;

    while ((end = strstr(end, NEWLINE))) {
        int len = end-begin;

        draw_textn(image, startx, starty, width, begin, len, factor);
        end += sizeof(NEWLINE)-1;
        begin = end;
        starty += line_space;
    }

    draw_textn(image, startx, starty, width, begin, strlen(begin), factor);

    return 0;
}

/**
 * initialize_chars
 */
int initialize_chars(void)
{
    unsigned int i;
    size_t draw_table_size;

    draw_table_size = sizeof(draw_table) / sizeof(struct draw_char);

    /* First init all char ptrs to a space character. */
    for (i = 0; i < ASCII_MAX; i++) {
        char_arr_ptr[i] = &draw_table[0].pix[0][0];
    }

    /* Build char_arr_ptr table to point to each available ascii. */
    for (i = 0; i < draw_table_size; i++) {
        char_arr_ptr[(int)draw_table[i].ascii] = &draw_table[i].pix[0][0];
    }

    return 0;
}

