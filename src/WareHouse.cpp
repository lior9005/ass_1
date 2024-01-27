#include "WareHouse.h"

WareHouse::WareHouse(const string &configFilePath) 
    : isOpen(false), customerCounter(0), volunteerCounter(0), orderCounter(0){
    
    // Open the file for reading
        std::ifstream file(configFilePath);

    // Check if the file is successfully opened
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << configFilePath << ". " << std::strerror(errno) << std::endl;
        return;
    }

    std::string line;
    // Read each line from the file
    while (std::getline(file, line)) {
    // If the line starts with '#' or empty, ignore it
        if (line.empty()) {
        continue;
        }
        if (line[0] == '#') {
        continue;
        }
        // Tokenize the line
        std::istringstream iss(line);
        std::vector<std::string> tokens{
            std::istream_iterator<std::string>{iss},
            std::istream_iterator<std::string>{}
        };

        // Check the type of entry (customer or volunteer)
        if (tokens[0] == "customer") {
            customerCounter++;

            // Check if the customer is a soldier
            if (tokens[2] == "soldier") {
                SoldierCustomer *newCustomer = new SoldierCustomer{
                    customerCounter,                // id
                    tokens[1],                     // name
                    std::stoi(tokens[3]),          // distance
                    std::stoi(tokens[4])           // max_orders
                };
                customers.push_back(newCustomer);
            }
            else {
                CivilianCustomer *newCustomer = new CivilianCustomer{
                    customerCounter,                // id
                    tokens[1],                     // name
                    std::stoi(tokens[3]),          // distance
                    std::stoi(tokens[4])           // max_orders
                };
                customers.push_back(newCustomer);
            }

        }

        else {
            volunteerCounter++;
            // Parse volunteer information

            if (tokens[2] == "collector") {
                CollectorVolunteer *newVolunteer = new CollectorVolunteer {
                    volunteerCounter,               // id
                    tokens[1],                     // name
                    std::stoi(tokens[3])           // cool_down
                };
                volunteers.push_back(newVolunteer);
            }
            else if (tokens[2] == "limited_collector") {
                LimitedCollectorVolunteer *newVolunteer = new LimitedCollectorVolunteer{
                    volunteerCounter,               // id
                    tokens[1],                     // name
                    std::stoi(tokens[3]),          // cool_down
                    std::stoi(tokens[4])           // max_orders
                };
                volunteers.push_back(newVolunteer);
            }
            else if (tokens[2] == "driver") {
                DriverVolunteer *newVolunteer = new DriverVolunteer{
                    volunteerCounter,               // id
                    tokens[1],                     // name
                    std::stoi(tokens[3]),          // max_distance                   
                    std::stoi(tokens[4])           // distance per step
                };
                volunteers.push_back(newVolunteer);
            }
            else{
                LimitedDriverVolunteer *newVolunteer = new LimitedDriverVolunteer{
                    volunteerCounter,               // id
                    tokens[1],                     // name
                    std::stoi(tokens[3]),          // max_distance
                    std::stoi(tokens[4]),          // distance per step
                    std::stoi(tokens[5])           // max_orders
                };
                volunteers.push_back(newVolunteer);
            }
        }
    }
    // Close the file
    file.close();
}

WareHouse::WareHouse(const WareHouse &other) 
    : isOpen(other.isOpen), customerCounter(other.customerCounter), volunteerCounter(other.volunteerCounter), orderCounter(orderCounter){
    for (Order* order : other.pendingOrders) {
        pendingOrders.push_back(order->clone());
    }
    for (Order* order : other.inProcessOrders) {
        inProcessOrders.push_back(order->clone());
    }
    for (Order* order : other.completedOrders) {
        completedOrders.push_back(order->clone());
    }
    for (BaseAction* action : other.actionsLog) {
        actionsLog.push_back(action->clone());
    }
    for (Volunteer* volunteer : other.volunteers) {
        volunteers.push_back(volunteer->clone());
    }
    for (Customer* customer : other.customers) {
        customers.push_back(customer->clone());
    }
}

WareHouse::WareHouse(WareHouse&& other) 
    : isOpen(other.isOpen), customerCounter(other.customerCounter), volunteerCounter(other.volunteerCounter),
      orderCounter(other.orderCounter), actionsLog(std::move(other.actionsLog)), volunteers(std::move(other.volunteers)),
      pendingOrders(std::move(other.pendingOrders)), inProcessOrders(std::move(other.inProcessOrders)),
      completedOrders(std::move(other.completedOrders)), customers(std::move(other.customers)) {
        other.actionsLog.clear();
        other.volunteers.clear();
        other.pendingOrders.clear();
        other.inProcessOrders.clear();
        other.completedOrders.clear();
        other.customers.clear();
    }
 
void WareHouse::start() {
    open();
}

const vector<BaseAction*>& WareHouse::getActions() const {
    return actionsLog;
}

void WareHouse::addOrder(Order* order) {
    pendingOrders.push_back(order);
    Customer *customer = customers[order->getCustomerId()];
    customer->addOrder(order->getId());
    orderCounter++;
}

void WareHouse::addCustomer(Customer* customer) {
    customers.push_back(customer);
    customerCounter++;
}

void WareHouse::addVolunteer(Volunteer* volunteer) {
    volunteers.push_back(volunteer);
    volunteerCounter++;
}

void WareHouse::addAction(BaseAction* action) {
    actionsLog.push_back(action);
}

const Customer& WareHouse::getCustomer(int customerId) const {
    return *customers[customerId];
}

const Volunteer& WareHouse::getVolunteer(int volunteerId) const {
    for (Volunteer* volunteer : volunteers) {
        if (volunteer->getId() == volunteerId) {
            return *volunteer;
        }
    }
}

const Order &WareHouse::getOrder(int orderId) const {
    for (Order* order : pendingOrders) {
        if (order->getId() == orderId) {
            return *order;
        }
    }
    for (Order* order : inProcessOrders) {
        if (order->getId() == orderId) {
            return *order;
        }
    }
    for (Order* order : completedOrders) {
        if (order->getId() == orderId) {
            return *order;
        }
    }
}

void WareHouse::advanceOrder(Order* order){
    if(order->getStatus() == OrderStatus::PENDING){
        auto it = pendingOrders.begin();
        while (it != pendingOrders.end() && *it != order) {
            it++;
        }
        pendingOrders.erase(it);
        order->setStatus(OrderStatus::COLLECTING);
        inProcessOrders.push_back(order);
    }
    else if(order->getStatus() == OrderStatus::COLLECTING){
        auto it = pendingOrders.begin();
        while (it != pendingOrders.end() && *it != order) {
            it++;
        }
        if (it != pendingOrders.end()) {
            pendingOrders.erase(it);
            inProcessOrders.push_back(order);
        }
        order->setStatus(OrderStatus::DELIVERING);
    }
    else{
        auto it = inProcessOrders.begin();
        while (it != inProcessOrders.end() && *it != order) {
            it++;
        }
        inProcessOrders.erase(it);
        completedOrders.push_back(order);
        order->setStatus(OrderStatus::COMPLETED);
    }
}

void WareHouse::deleteVolunteer(Volunteer* volunteer){
        auto it = volunteers.begin();
        while (it != volunteers.end() && *it != volunteer) {
            it++;
        }
        volunteers.erase(it);
}

void WareHouse::close() {
    isOpen = false;
    delete this;
}

void WareHouse::open() {
    isOpen = true;
    std::cout << "Warehouse is open!" << std::endl;
    while (isOpen) {
        std::string input;
        std::getline(std::cin, input);  // save the string entered in input
        std::istringstream iss(input);  // Split the input string into words
        std::vector<std::string> words;
        std::string word;
        while (iss >> word) {
            words.push_back(word);
        }

        if (words[0] == "step") {
            BaseAction* step = new SimulateStep(std::stoi(words[1]));
            step->act(*this);
            addAction(step);
        }
        else if (words[0] == "order") {
            BaseAction* addOrder = new AddOrder(std::stoi(words[1]));
            addOrder->act(*this);
            addAction(addOrder);
        }
        else if (words[0] == "customer") {
            BaseAction* addCustomer = new AddCustomer(words[1], words[2], std::stoi(words[3]), std::stoi(words[4]));
            addCustomer->act(*this);
            addAction(addCustomer);
        }
        else if (words[0] == "orderStatus") {
            BaseAction* orderStatus = new PrintOrderStatus(std::stoi(words[1]));
            orderStatus->act(*this);
            addAction(orderStatus);          
        }
        else if (words[0] == "customerStatus") {
            BaseAction* customerStatus = new PrintCustomerStatus(std::stoi(words[1]));
            customerStatus->act(*this);
            addAction(customerStatus);            
        }
        else if (words[0] == "volunteerStatus") {
            BaseAction* volunteerStatus = new PrintVolunteerStatus(std::stoi(words[1]));
            volunteerStatus->act(*this);
            addAction(volunteerStatus);            
        }
        else if (words[0] == "log") {
            BaseAction* log = new PrintActionsLog();
            log->act(*this);
            addAction(log);            
        }
        else if (words[0] == "close") {
            BaseAction* close = new Close();
            close->act(*this);
            addAction(close);            
        }
        else if (words[0] == "backup") {
            BaseAction* backup = new BackupWareHouse();
            backup->act(*this);
            addAction(backup);            
        }
        else if (words[0] == "restore") {
            BaseAction* restore = new RestoreWareHouse();
            restore->act(*this);
            addAction(restore);            
        }
        else {
            std::cout << "Incorrect Input" << std::endl;
        }
    }
}

WareHouse::~WareHouse() {
    for (BaseAction* action : actionsLog) {
        delete(action);
    }
    for (Volunteer* volunteer : volunteers) {
        delete(volunteer);
    }    
    for (Order* order : pendingOrders) {
        delete(order);
    }
    for (Order* order : inProcessOrders) {
        delete(order);
    }
    for (Order* order : completedOrders) {
        delete(order);
    }
    for (Customer* customer : customers) {
        delete(customer);
    }
}

WareHouse& WareHouse::operator=(const WareHouse& other){
    if(&other != this){
        isOpen = other.isOpen;
        customerCounter = other.customerCounter;
        volunteerCounter = other.volunteerCounter;
        orderCounter = other.orderCounter;
        for (BaseAction* action : actionsLog) {
            delete(action);
        }
        actionsLog.clear();
        for (BaseAction* action : other.actionsLog) {
            actionsLog.push_back(action->clone());
        }
        for (Volunteer* volunteer : volunteers) {
            delete(volunteer);
        }
        volunteers.clear();
        for (Volunteer* volunteer : other.volunteers) {
            volunteers.push_back(volunteer->clone());
        }
        for (Order* order : pendingOrders) {
            delete(order);
        }
        pendingOrders.clear();
        for (Order* order : other.pendingOrders) {
            pendingOrders.push_back(order->clone());
        }
        for (Order* order : inProcessOrders) {
            delete(order);
        }
        inProcessOrders.clear();
        for (Order* order : other.inProcessOrders) {
            inProcessOrders.push_back(order->clone());
        }
        for (Order* order : completedOrders) {
            delete(order);
        }
        completedOrders.clear();
        for (Order* order : other.completedOrders) {
            completedOrders.push_back(order->clone());
        }
        for (Customer* customer : customers) {
            delete(customer);
        }
        actionsLog.clear();
        for (Customer* customer : other.customers) {
            customers.push_back(customer->clone());
        }       
    }
    return *this;
}

WareHouse& WareHouse::operator=(WareHouse&& other){
    if(&other != this){
        isOpen = other.isOpen;
        customerCounter = other.customerCounter;
        volunteerCounter = other.volunteerCounter;
        orderCounter = other.orderCounter;

        for (BaseAction* action : actionsLog) {
            delete action;
        }
        for (Volunteer* volunteer : volunteers) {
            delete volunteer;
        }
        for (Order* order : pendingOrders) {
            delete order;
        }
        for (Order* order : inProcessOrders) {
            delete order;
        }
        for (Order* order : completedOrders) {
            delete order;
        }
        for (Customer* customer : customers) {
            delete customer;
        }

        actionsLog = std::move(other.actionsLog);
        volunteers = std::move(other.volunteers);
        pendingOrders = std::move(other.pendingOrders);
        inProcessOrders = std::move(other.inProcessOrders);
        completedOrders = std::move(other.completedOrders);
        customers = std::move(other.customers);

        other.actionsLog.clear();
        other.volunteers.clear();
        other.pendingOrders.clear();
        other.inProcessOrders.clear();
        other.completedOrders.clear();
        other.customers.clear();  
    }
    return *this;
}


void WareHouse:: printAllCustomers(){
    for (Customer* customer : customers) {
        cout << customer->toString() << endl;
    }
}
void WareHouse:: printAllVolunteers(){
    for (Volunteer* volunteer : volunteers) {
        cout << volunteer->toString() << endl;
    }
}

void WareHouse:: printAllOrders(){
    for (Order* order : pendingOrders) {
        cout << "OrderID:" << order->getId() << ", CustomerID: " << order->getCustomerId() << ", OrderStatus: " << order->getStatusString() << endl;
    }
}