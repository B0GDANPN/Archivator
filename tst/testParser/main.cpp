#include "../../src/controller/Controller.h"

int main() {
    std::vector<std::string> argv = {"enc -o 500 -f Lena.bmp",
                                     "enc -f vid.mp4",
                                     "fdfd", "trash", "enc -f",
                                     "end",
                                     "gfdgffgf",
                                     "", "-o",
                                     "dec -o 5 -f Lena.bmp",
                                     "testTxt1.txt", "nonexist.txt",
                                     "testTxt2.txt"};
    std::stringstream ss;
    for (size_t i = 0; i < argv.size(); ++i) {
        if (i != 0)
            ss << '\n';
        ss << argv[i];
    }
    std::string str = ss.str();
    std::vector<Dto> args = Parser::parse(str);
    //assert(args.size()==3);
    std::vector<Dto> expected;
    expected.emplace_back(Dto{true, {"500"}, {"Lena.bmp"}});
    expected.emplace_back(Dto{true, {}, {"vid.mp4"}});
    expected.emplace_back(Dto{false, {"5"}, {"Lena.bmp"}});

    assert(args.size() == expected.size());
    for (int i = 0; i < args.size(); ++i) {
        assert(args[i] == expected[i]);
    }
    //assert(args[0].action_= true);
    //assert(args[1].action_= true);
    //assert(args[2].action_= false);

    //assert(args[0].options_.size()==1);
    //assert(args[1].action_= true);
    //assert(args[2].action_= false);
}