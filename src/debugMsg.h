#ifndef DEBUGMSG_H_
#define DEBUGMSG_H_

#ifndef DO_DEBUG
#define _DO_DEBUG false
#endif
#ifdef DO_DEBUG
#define _DO_DEBUG true
#endif

#define DBG_STREAM if (!_DO_DEBUG) {} else std::cerr
#endif // DEBUGMSG_H_
