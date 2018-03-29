char *str_trim_head(char *str)
{
    while(*str==' ' || *str == '\t' || *str == '\r' || *str == '\n') {
        str++;
    }

    return str;
}
