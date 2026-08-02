
#ifndef APPLICATION_DEFINES_HPP
#define APPLICATION_DEFINES_HPP

enum class eGameState : std::uint32_t
{
    none        = 0,
    initialize  = 1,
    process     = 2,
    terminate   = 3
};

enum class eGameMode : std::uint32_t
{
    none = 0,
    menu = 1,
    play = 2,
    edit = 3,
    win = 4
};

#endif // APPLICATION_DEFINES_HPP
