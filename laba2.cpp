#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <ctime>
enum class TariffType {
    ECONOMY,
    STANDARD,
    PREMIUM,
    UNLIMITED
};

class Tariff {
private:
    int id;
    std::string name;
    TariffType type;
    double pricePerGB;
    double monthlyFee;

public:
    Tariff(int id, const std::string& name, TariffType type, double pricePerGB, double monthlyFee)
        : id(id), name(name), type(type), pricePerGB(pricePerGB), monthlyFee(monthlyFee) {
    }

    int getId() const { return id; }
    std::string getName() const { return name; }
    TariffType getType() const { return type; }
    double getPricePerGB() const { return pricePerGB; }
    double getMonthlyFee() const { return monthlyFee; }

    std::string getTypeString() const {
        switch (type) {
        case TariffType::ECONOMY: return "Эконом";
        case TariffType::STANDARD: return "Стандарт";
        case TariffType::PREMIUM: return "Премиум";
        case TariffType::UNLIMITED: return "Безлимит";
        default: return "Неизвестно";
        }
    }

    void display() const {
        std::cout << "ID: " << id << "\n";
        std::cout << "Название: " << name << "\n";
        std::cout << "Тип: " << getTypeString() << "\n";
        std::cout << "Цена за 1 ГБ: " << std::fixed << std::setprecision(2) << pricePerGB << " руб.\n";
        std::cout << "Абонентская плата: " << monthlyFee << " руб./мес.\n";
    }
};

class Client {
private:
    int id;
    std::string name;
    std::string address;
    std::string phone;
    std::time_t registrationDate;
    int tariffId;
    double trafficUsed;
    std::vector<std::pair<std::time_t, double>> trafficHistory;

public:
    Client(int id, const std::string& name, const std::string& address,
        const std::string& phone, int tariffId)
        : id(id), name(name), address(address), phone(phone),
        tariffId(tariffId), trafficUsed(0) {
        registrationDate = std::time(nullptr);
    }

    int getId() const { return id; }
    std::string getName() const { return name; }
    std::string getAddress() const { return address; }
    std::string getPhone() const { return phone; }
    std::time_t getRegistrationDate() const { return registrationDate; }
    int getTariffId() const { return tariffId; }
    double getTrafficUsed() const { return trafficUsed; }
    const std::vector<std::pair<std::time_t, double>>& getTrafficHistory() const { return trafficHistory; }

    void addTraffic(double trafficGB) {
        if (trafficGB < 0) {
            throw std::invalid_argument("Объем трафика не может быть отрицательным");
        }
        trafficUsed += trafficGB;
        trafficHistory.push_back(std::make_pair(std::time(nullptr), trafficGB));
    }

    double calculateCost(const Tariff& tariff) const {
        double trafficCost = trafficUsed * tariff.getPricePerGB();
        return trafficCost + tariff.getMonthlyFee();
    }

    void display() const {
        std::cout << "ID: " << id << "\n";
        std::cout << "Имя: " << name << "\n";
        std::cout << "Адрес: " << address << "\n";
        std::cout << "Телефон: " << phone << "\n";

        char buffer[26];
        ctime_s(buffer, sizeof(buffer), &registrationDate);
        std::cout << "Дата регистрации: " << buffer;

        std::cout << "ID тарифа: " << tariffId << "\n";
        std::cout << "Использовано трафика: " << std::fixed << std::setprecision(2)
            << trafficUsed << " ГБ\n";
    }

    void displayTrafficHistory() const {
        std::cout << "История использования трафика для клиента " << name << ":\n";
        for (const auto& record : trafficHistory) {
            char buffer[26];
            ctime_s(buffer, sizeof(buffer), &record.first);
            std::cout << "  Дата: " << buffer
                << "  Трафик: " << record.second << " ГБ\n";
        }
    }
};

class ProviderSystem {
private:
    static ProviderSystem* instance;

    std::vector<Tariff> tariffs;
    std::vector<Client> clients;
    int nextClientId;
    int nextTariffId;

    ProviderSystem() : nextClientId(1), nextTariffId(1) {
        addTariff("Экономный", TariffType::ECONOMY, 50.0, 300.0);
        addTariff("Стандартный", TariffType::STANDARD, 40.0, 500.0);
        addTariff("Премиум", TariffType::PREMIUM, 30.0, 800.0);
        addTariff("Безлимит", TariffType::UNLIMITED, 0.0, 1200.0);
    }

    ProviderSystem(const ProviderSystem&) = delete;
    ProviderSystem& operator=(const ProviderSystem&) = delete;

public:
    static ProviderSystem* getInstance() {
        if (instance == nullptr) {
            instance = new ProviderSystem();
        }
        return instance;
    }

    ~ProviderSystem() {
        tariffs.clear();
        clients.clear();
    }

    void addTariff(const std::string& name, TariffType type, double pricePerGB, double monthlyFee) {
        tariffs.emplace_back(nextTariffId++, name, type, pricePerGB, monthlyFee);
    }

    void registerClient(const std::string& name, const std::string& address,
        const std::string& phone, int tariffId) {
        clients.emplace_back(nextClientId++, name, address, phone, tariffId);
    }

    bool addTrafficToClient(int clientId, double trafficGB) {
        auto it = std::find_if(clients.begin(), clients.end(),
            [clientId](const Client& c) { return c.getId() == clientId; });

        if (it != clients.end()) {
            it->addTraffic(trafficGB);
            return true;
        }
        return false;
    }

    double calculateTotalRevenue() const {
        double total = 0.0;
        for (const auto& client : clients) {
            const Tariff* tariff = getTariffById(client.getTariffId());
            if (tariff) {
                total += client.calculateCost(*tariff);
            }
        }
        return total;
    }

    Client* findClientWithMaxPayment() {
        if (clients.empty()) return nullptr;

        double maxPayment = 0.0;
        Client* maxClient = nullptr;

        for (auto& client : clients) {
            const Tariff* tariff = getTariffById(client.getTariffId());
            if (tariff) {
                double payment = client.calculateCost(*tariff);
                if (payment > maxPayment) {
                    maxPayment = payment;
                    maxClient = &client;
                }
            }
        }

        return maxClient;
    }

    Tariff* getTariffById(int id) {
        auto it = std::find_if(tariffs.begin(), tariffs.end(),
            [id](const Tariff& t) { return t.getId() == id; });

        return (it != tariffs.end()) ? &(*it) : nullptr;
    }

    const Tariff* getTariffById(int id) const {
        auto it = std::find_if(tariffs.begin(), tariffs.end(),
            [id](const Tariff& t) { return t.getId() == id; });

        return (it != tariffs.end()) ? &(*it) : nullptr;
    }

    Client* getClientById(int id) {
        auto it = std::find_if(clients.begin(), clients.end(),
            [id](const Client& c) { return c.getId() == id; });

        return (it != clients.end()) ? &(*it) : nullptr;
    }

    const std::vector<Tariff>& getTariffs() const { return tariffs; }

    const std::vector<Client>& getClients() const { return clients; }

    void displayAllTariffs() const {
        std::cout << "\n=== Доступные тарифы ===\n";
        for (const auto& tariff : tariffs) {
            tariff.display();
            std::cout << "------------------------\n";
        }
    }

    void displayAllClients() const {
        std::cout << "\n=== Зарегистрированные клиенты ===\n";
        for (const auto& client : clients) {
            client.display();
            const Tariff* tariff = getTariffById(client.getTariffId());
            if (tariff) {
                std::cout << "Стоимость услуг: " << std::fixed << std::setprecision(2)
                    << client.calculateCost(*tariff) << " руб.\n";
            }
            std::cout << "------------------------\n";
        }
    }
};

ProviderSystem* ProviderSystem::instance = nullptr;

static int inputInt(const std::string& prompt, int min = 0, int max = 1000000) {
    int value;
    std::string line;

    while (true) {
        std::cout << prompt;
        std::getline(std::cin, line);

        std::istringstream iss(line);
        if (iss >> value) {
            char remaining;
            if (!(iss >> remaining)) {
                if (value >= min && value <= max) {
                    return value;
                }
            }
        }

        std::cout << "Ошибка ввода! Введите целое число от " << min << " до " << max << "\n";
    }
}

static double inputDouble(const std::string& prompt, double min = 0.0, int max = 200) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= min && value <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cout << "Ошибка ввода! Введите число от " << min << " до " << max << "\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

static std::string inputString(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return value;
}

static TariffType inputTariffType() {
    int choice;
    std::cout << "\nВыберите тип тарифа:\n";
    std::cout << "1. Эконом\n";
    std::cout << "2. Стандарт\n";
    std::cout << "3. Премиум\n";
    std::cout << "4. Безлимит\n";

    choice = inputInt("Ваш выбор (1-4): ", 1, 4);

    switch (choice) {
    case 1: return TariffType::ECONOMY;
    case 2: return TariffType::STANDARD;
    case 3: return TariffType::PREMIUM;
    case 4: return TariffType::UNLIMITED;
    default: return TariffType::STANDARD;
    }
}

static void showMenu() {
    std::cout << "\n=== Система управления интернет-провайдером ===\n";
    std::cout << "1. Показать все тарифы\n";
    std::cout << "2. Добавить новый тариф\n";
    std::cout << "3. Показать всех клиентов\n";
    std::cout << "4. Зарегистрировать нового клиента\n";
    std::cout << "5. Добавить использованный трафик клиенту\n";
    std::cout << "6. Показать историю трафика клиента\n";
    std::cout << "7. Рассчитать общую выручку\n";
    std::cout << "8. Найти клиента с максимальной оплатой\n";
    std::cout << "9. Выход\n";
}

int main() {
    system("chcp 1251 > nul");


    ProviderSystem* system = ProviderSystem::getInstance();

    int choice;
    bool running = true;

    while (running) {
        showMenu();
        choice = inputInt("Выберите действие (1-9): ", 1, 9);

        switch (choice) {
        case 1: {
            system->displayAllTariffs();
            break;
        }
        case 2: {
            std::string name = inputString("Введите название тарифа: ");
            TariffType type = inputTariffType();
            double pricePerGB = inputDouble("Введите цену за 1 ГБ (руб.): ");
            double monthlyFee = inputDouble("Введите абонентскую плату (руб./мес.): ", 200, 1500);

            system->addTariff(name, type, pricePerGB, monthlyFee);
            std::cout << "Тариф успешно добавлен!\n";
            break;
        }
        case 3: {
            system->displayAllClients();
            break;
        }
        case 4: {
            std::string name = inputString("Введите ФИО клиента: ");
            std::string address = inputString("Введите адрес: ");
            std::string phone = inputString("Введите телефон: ");

            system->displayAllTariffs();
            int tariffId = inputInt("Введите ID тарифа для клиента: ");

            const Tariff* tariff = system->getTariffById(tariffId);
            if (tariff) {
                system->registerClient(name, address, phone, tariffId);
                std::cout << "Клиент успешно зарегистрирован!\n";
            }
            else {
                std::cout << "Ошибка: тариф с таким ID не найден!\n";
            }
            break;
        }
        case 5: {
            system->displayAllClients();
            if (!system->getClients().empty()) {
                int clientId = inputInt("Введите ID клиента: ");
                double traffic = inputDouble("Введите объем использованного трафика (ГБ): ");

                if (system->addTrafficToClient(clientId, traffic)) {
                    std::cout << "Трафик успешно добавлен!\n";
                }
                else {
                    std::cout << "Ошибка: клиент с таким ID не найден!\n";
                }
            }
            break;
        }
        case 6: {
            system->displayAllClients();
            if (!system->getClients().empty()) {
                int clientId = inputInt("Введите ID клиента для просмотра истории: ");
                Client* client = system->getClientById(clientId);
                if (client) {
                    client->displayTrafficHistory();
                }
                else {
                    std::cout << "Ошибка: клиент с таким ID не найден!\n";
                }
            }
            break;
        }
        case 7: {
            double total = system->calculateTotalRevenue();
            std::cout << "\nОбщая выручка от всех клиентов: "
                << std::fixed << std::setprecision(2)
                << total << " руб.\n";
            break;
        }
        case 8: {
            Client* maxClient = system->findClientWithMaxPayment();
            if (maxClient) {
                const Tariff* tariff = system->getTariffById(maxClient->getTariffId());
                if (tariff) {
                    std::cout << "\nКлиент с максимальной оплатой:\n";
                    maxClient->display();
                    std::cout << "Тариф: " << tariff->getName() << "\n";
                    std::cout << "Общая стоимость услуг: "
                        << std::fixed << std::setprecision(2)
                        << maxClient->calculateCost(*tariff) << " руб.\n";
                }
            }
            else {
                std::cout << "Нет зарегистрированных клиентов!\n";
            }
            break;
        }
        case 9: {
            running = false;
            std::cout << "Выход из программы...\n";
            delete system;
            break;
        }
        }
    }

    return 0;
}