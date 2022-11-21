#ifndef RETURN_CODE_HPP_INCLUDED
#define RETURN_CODE_HPP_INCLUDED








enum  Return_code  {

    SUCCESS    = 0,
    MEMORY_ERR = 1,
    BAD_ARGS   = 2,
    FILE_ERR   = 3,
};



#define LOG_ERROR(code) _log_error (code,    __FILE__, __PRETTY_FUNCTION__, __LINE__)

#define try(x, ...) {\
\
    Return_code _return_code_try_special = (x);\
    if (_return_code_try_special) { LOG_ERROR (_return_code_try_special); return _return_code_try_special; }\
}

#define try_return(x, return_value) {\
\
    Return_code _return_code_try_special = (x);\
    if (_return_code_try_special) { LOG_ERROR (_return_code_try_special); return return_value; }\
}


//--------------------------------------------------
//help macros vsc
#endif