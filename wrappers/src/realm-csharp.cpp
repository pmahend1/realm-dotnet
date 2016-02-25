/* Copyright 2015 Realm Inc - All Rights Reserved
 * Proprietary and Confidential
 */
 
#include <iostream>
#include <functional>
#include <list>
#include "object-store/src/binding_context.hpp"
#include "error_handling.hpp"

using namespace realm;

namespace realm {
namespace binding {

class CSharpRealmDelegate: public BindingContext {
public:
    // A token returned from add_notification that can be used to remove the
    // notification later
    struct token : private std::list<std::function<void()>>::iterator {
        token(std::list<std::function<void()>>::iterator it) : std::list<std::function<void()>>::iterator(it) { }
        friend class CSharpRealmDelegate;
    };

    token add_notification(std::function<void()> func)
    {
        m_registered_notifications.push_back(std::move(func));
        return token(std::prev(m_registered_notifications.end()));
    }

    void remove_notification(token entry)
    {
        m_registered_notifications.erase(entry);
    }

    // Override the did_change method to call each registered notification
    void did_change(std::vector<ObserverState> const&, std::vector<void*> const&) override
    {
        // Loop oddly so that unregistering a notification from within the
        // registered function works
        for (auto it = m_registered_notifications.begin(); it != m_registered_notifications.end(); ) {
            (*it++)();
        }
    }

private:
    std::list<std::function<void()>> m_registered_notifications;
};

CSharpRealmDelegate* delegate_instance = new CSharpRealmDelegate();

}
}

#ifdef DYNAMIC  // clang complains when making a dylib if there is no main(). :-/
int main() { return 0; }
#endif