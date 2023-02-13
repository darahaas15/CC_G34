// Single line comment

/* Multi line comment */

#def TEN 10
    // Prints 10
    dbg TEN;

/* #def STATEMENTS let a = 5; \ let x = a * 16; \
    let b = 10; 
    STATEMENTS;
    dbg a + b * x; */

#def NOBODY
    // Prints 1
    dbg NOBODY;

#def ABC 4
#def DEF ABC + 5
#def GHI ABC * DEF
    // Prints 4 * 4 + 5 = 21
    dbg GHI; 

#def ABC 1
    dbg ABC;
#def ABC 15
    dbg ABC;

#def ABC 2
// #undef ABC
    // Throws an error
    dbg ABC;