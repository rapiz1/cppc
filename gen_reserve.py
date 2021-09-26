f = open('token.h', 'r')
reserved = []
scan = False
for line in f:
    if 'TEOF' in line:
        scan = False
    if scan:
        word = line.strip().split(',')[0]
        if len(word):
            reserved.append(word)
    if 'Keyword' in line:
        scan = True
template = '''
#include "token.h"

#include <map>
#include <string>
const static std::map<std::string, TokenType> str2token = {{
  {}
}};

'''
output = ''
for x in reserved:
    output += f'{{ "{x.lower()}", {x} }},'
with open('token_convert.h', 'w') as of:
    of.write(template.format(output))
