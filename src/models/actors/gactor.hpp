// Represents a GTSActor
// Holds a reference and convience fuctions as well as data for that actor
//
// It's parent classes should contain the needed modules such as crush/vore/grow
//
// This GActor itself should be subclassed for specific actors such as Player
// in models/personas this will allow for override of abilities for certain actors
//

#include "models/serde.hpp"

class GActor: Public Serde {
    
};
