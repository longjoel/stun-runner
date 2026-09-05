#include <iostream>

namespace stunrun::smoke {

int classify(int value)
{
    return value > 0 ? 1 : 0;
}

} // namespace stunrun::smoke

int main()
{
    if (stunrun::smoke::classify(1) != 1 || stunrun::smoke::classify(0) != 0)
        return 1;
    std::cout << "native smoke ok\n";
    return 0;
}
