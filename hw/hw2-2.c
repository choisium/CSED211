long cread(long *xp) {
    return (xp ? *xp : 0);
}

long cread_alt(long *xp) {
    // if (!xp) return 0;
    // else return *xp;
    return (!xp? 0 : *xp);
}