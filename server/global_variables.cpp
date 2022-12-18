#include "global_variables.h"
//实现定义
const uint32_t CONNEVENT = EPOLLONESHOT | EPOLLRDHUP | EPOLLET | EPOLLIN;
const uint32_t LISTENEVENT = EPOLLRDHUP | EPOLLET | EPOLLIN;

const int port1 = 9000;
const int port2 = 8000;
const int port3 = 10000;
const unordered_map<string, int> cmdmap =
{
    {"register",0},
    {"login",1},
    {"search",2},
    {"chat",3},
    {"accept",4},
    {"reject",5},
    {"break",6},
    {"send",7},
    {"sendfile",8},
    {"acceptfile",9},
    {"rejectfile",10},
    {"getfile",11},
    {"acceptget",12},
    {"rejectget",13},
    {"setsid",14},
    {"setsname",15},
    {"choosefile",16},//服务器没有re命令，但是多了一个choosefile，通告选择的文件号码
    {"exit",17},
    {"hisir",18},
    {"myresource",19},//这个也不用
    {"oyasumi",20}
};

const vector<string> hisir_sentence = { "Snowflakes are dreams that fall on a winter day",
"If the breath is touched by the wind, it will be taken away",
"The ideal exists outside the light cone",
"Only by continuing to think is we truly aware of existence itself",
"It is right to separate from the world",
"Integration into the collective, integration into society, retention of self-preservation thinking",
 "Emotions are based on material existence",
"AI is in some ways better than humans not only because they learn faster, but also because they don't have emotions",
"Now is a continuation of the facts of the past, but not of the meaning of the past",
"Countless times I want to glimpse the world after death and see how it differs from the world before death",
"What kind of person do you want to be?"
"How long before you get out of the limits of a Turing machine",
"Incalculable things, sincerely feel beautiful",
"If you don't stop, you can't decide, the cycle is the end of fate",
"Light in society, I prefer to understand as protecting the victims of endless darkness",
"Human individuals and groups are very different, you can always have hope for a person",
"Thinking and emotions give human beings a code of action other than instinct, but it is also human nature that sometimes becomes the culprit of the ugliest things in the world",
"Everyone has their own 'truth', it is born from a narrow worldview, it is a modified information to protect itself, it is a distorted truth",
"If you feel that the world is wrong, maybe you have the wrong perception of the world",
"Sunny days are cheerful, rainy days are melancholy. When people's cognition is portrayed in this way, they are convinced. In fact, happy things happen on rainy days",
"Emotional intelligence is an absolutely useful thing",
"Trust and doubt are crucial",
"Follow the sound, you can meet",
"Hello~",
"Thank you letter",
"There won't be a bug",
"do not evaluate"};

const vector<string> oyasumi_sentence = { "What kind of melody will sound in your head at this moment",
"Good night~",
"Solitude is not solitude, it is a gift of divine reward",
"oyasumi~",
"Will you have something to regret",
"The beauty of all life becomes a 'Merry Christmas', \nand then white snowflakes fly all over the sky, falling into every clean and unclean corner of this world"};

