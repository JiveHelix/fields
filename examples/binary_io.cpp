#include <fields/describe.h>
#include <fields/binary_io.h>
#include <sstream>
#include <iostream>


struct HasOptionals
{
    int foo;
    std::optional<int> bar;
    std::optional<short> czar;

#if 0
    static constexpr auto fields = std::make_tuple(
        fields::Field(&HasOptionals::foo, "foo"),
        fields::Field(&HasOptionals::bar, "bar"),
        fields::Field(&HasOptionals::czar, "czar"));
#endif
};


int main()
{
    HasOptionals data{};
    data.foo = 42;
    data.czar = short(7);

    std::stringstream ss;
    fields::Write(ss, data);

    std::cout << std::hex;

    for (auto c: ss.str())
    {
        std::cout << uint16_t(c) << " ";
    }

    std::cout << std::dec << '\n';

    ss.seekg(0);

    auto recovered = fields::Read<HasOptionals>(ss);

    std::cout << fields::DescribeColorized(recovered, 1) << std::endl;

    return 0;
}
