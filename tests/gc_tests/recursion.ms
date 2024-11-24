fun fib(n) {
    a = ["Some long string to fill the memory.", """
    Alvy: I feel that life is divided into the horrible and the miserable.
          That's the two categories. The horrible are like, I don't know,
          terminal cases, you know, and blind people, crippled. I don't know
          how they get through life. It's amazing to me. And the miserable is
          everyone else. So you should be thankful that you're miserable,
          because that's very lucky, to be miserable.
    """,
    "I think crime pays. The hours are good, you meet a lot of interesting people, you travel a lot.",
    "I'm astounded by people who want to 'know' the universe when it's hard enough to find your way around Chinatown.",
    """
    In my next life I want to live my life backwards. You start out dead and
    get that out of the way. Then you wake up in an old people's home feeling
    better every day. You get kicked out for being too healthy, go collect your
    pension, and then when you start work, you get a gold watch and a party on
    your first day. You work for 40 years until you're young enough to enjoy
    your retirement. You party, drink alcohol, and are generally promiscuous,
    then you are ready for high school. You then go to primary school, you
    become a kid, you play. You have no responsibilities, you become a baby
    until you are born. And then you spend your last 9 months floating in
    luxurious spa-like conditions with central heating and room service on tap,
    larger quarters every day and then Voila! You finish off as an or-asm!
    """]
    if(n < 2) return n
    else return fib(n-1) + fib(n-2)
}


fib(13)
"\n"