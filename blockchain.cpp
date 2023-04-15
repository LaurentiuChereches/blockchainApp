#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <openssl/sha.h>

// Clasa pentru tranzactii
class Transaction {
public:
    Transaction(const std::string& sender, const std::string& recipient, double amount)
        : m_sender(sender), m_recipient(recipient), m_amount(amount) {}

    std::string getSender() const { return m_sender; }
    std::string getRecipient() const { return m_recipient; }
    double getAmount() const { return m_amount; }

    bool isValid() const {
        // Verifica daca suma trimisa este mai mare decat 0
        if (m_amount <= 0.0) {
            return false;
        }

        // Verifica daca senderul are suficienti bani pentru a trimite suma
        double totalSent = 0.0;
        for (const auto& sent : m_senderSent) {
            totalSent += sent;
        }
        if (totalSent < m_amount) {
            return false;
        }

        return true;
    }

    void setSenderSent(const std::vector<double>& sent) {
        m_senderSent = sent;
    }

private:
    std::string m_sender;
    std::string m_recipient;
    double m_amount;
    std::vector<double> m_senderSent; // tine evidenta sumelor trimise de sender
};

// Clasa pentru portofel
class Wallet {
public:
    Wallet(const std::string& name, double balance = 0.0) : m_name(name), m_balance(balance) {}

    std::string getName() const { return m_name; }
    double getBalance() const { return m_balance; }

    std::vector<Transaction> sendMoney(double amount, const std::string& recipient) {
        std::vector<Transaction> transactions;
        double remainingAmount = amount;

        // Trimite bani catre destinatar
        if (m_balance >= amount) {
            transactions.push_back(Transaction(m_name, recipient, amount));
            m_balance -= amount;
            remainingAmount = 0.0;
        } else {
            transactions.push_back(Transaction(m_name, recipient, m_balance));
            remainingAmount -= m_balance;
            m_balance = 0.0;
        }

        // Trimite bani catre propria adresa
        if (remainingAmount > 0.0) {
            transactions.push_back(Transaction(m_name, m_name, remainingAmount));
        }

        // Actualizeaza evidenta sumelor trimise de portofelul expeditorului
        std::vector<double> senderSent;
        for (const auto& transaction : transactions) {
            // a fost portofelul nostru, adaugam sumele trimise la evidenta
            if (transaction.getSender() == m_name) {
                senderSent.push_back(transaction.getAmount());
            }
        }
        if (!senderSent.empty()) {
            transactions.back().setSenderSent(senderSent);
        }
        return transactions;
    }

    void receiveMoney(const std::vector<Transaction>& transactions) {
    // Adauga bani la soldul portofelului
        for (const auto& transaction : transactions) {
            if (transaction.getRecipient() == m_name) {
                m_balance += transaction.getAmount();
            }
        }
    }

private:
    std::string m_name;
    double m_balance;
};

class Block {
public:
    Block(const std::vector<Transaction>& transactions, const std::string& previousHash)
    : m_transactions(transactions), m_previousHash(previousHash), m_nonce(0), m_reward(50.0) {}

    std::string calculateHash() const {
        std::stringstream ss;
        ss << m_previousHash << m_nonce << m_reward;
        for (const auto& transaction : m_transactions) {
            ss << transaction.getSender() << transaction.getRecipient() << transaction.getAmount();
            for (const auto& sent : transaction.getSenderSent()) {
                ss << sent;
            }
        }
        return sha256(ss.str());
    }

    void mineBlock(int difficulty) {
        std::string target(difficulty, '0');
        while (m_hash.substr(0, difficulty) != target) {
            m_nonce++;
            m_hash = calculateHash();
        }
        std::cout << "Block mined: " << m_hash << std::endl;
    }

    std::vector<Transaction> getTransactions() const { return m_transactions; }
    std::string getPreviousHash() const { return m_previousHash; }
    std::string getHash() const { return m_hash; }
    double getReward() const { return m_reward; }

private:
    std::vector<Transaction> m_transactions;
    std::string m_previousHash;
    std::string m_hash;
    int m_nonce;
    double m_reward;

    std::string sha256(const std::string& str) const {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, str.c_str(), str.size());
        SHA256_Final(hash, &sha256);
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
};

class Blockchain {
public:
    Blockchain() {
        m_difficulty = 2;
        m_pendingTransactions.clear();
        m_chain.emplace_back(std::vector<Transaction>(), "0");
    }

    void addTransaction(const Transaction& transaction) {
        if (!transaction.isValid()) {
            std::cerr << "Invalid transaction" << std::endl;
            return;
        }

        m_pendingTransactions.push_back(transaction);
    }

    void minePendingTransactions(const std::string& miner) {
        std::vector<Transaction> transactions(m_pendingTransactions);
        transactions.emplace_back("SYSTEM", miner, m_chain.back().getReward());

        Block block(transactions, m_chain.back().getHash());
        block.mineBlock(m_difficulty);

        m_chain.push_back(block);
        m_pendingTransactions.clear();
    }

    bool isValid() const {
        for (unsigned int i = 1; i < m_chain.size(); ++i) {
            const Block& currentBlock = m_chain[i];
            const Block& previousBlock = m_chain[i - 1];

            // Verifica daca hash-ul blocului este valid
            if (currentBlock.getHash() != currentBlock.calculateHash()) {
                std::cerr << "Invalid block hash" << std::endl;
                return false;
            }

            // Verifica daca hash-ul anterior al blocului este corect
            if (currentBlock.getPreviousHash() != previousBlock.getHash()) {
                std::cerr << "Invalid previous block hash" << std::endl;
                return false;
            }

            // Verifica daca tranzactiile din bloc sunt valide
            for (const auto& transaction : currentBlock.getTransactions()) {
                if (!transaction.isValid()) {
                    std::cerr << "Invalid transaction in block" << std::endl;
                    return false;
                }
            }

            // Verifica daca minerul a primit recompensa
            if (currentBlock.getTransactions().back().getRecipient() != m_chain.back().getHash()) {
                std::cerr << "Miner reward not received" << std::endl;
                return false;
            }
        }
        return true;
    }

    void printChain() const {
        for (const auto& block : m_chain) {
            std::cout << "Block " << &block - &m_chain[0] << std::endl;
            std::cout << "Hash: " << block.getHash() << std::endl;
            std::cout << "Previous hash: " << block.getPreviousHash() << std::endl;
            std::cout << "Reward: " << block.getReward() << std::endl;
            std::cout << "Transactions:" << std::endl;
            for (const auto& transaction : block.getTransactions()) {
                std::cout << "  Sender: " << transaction.getSender() << std::endl;
                std::cout << "  Recipient: " << transaction.getRecipient() << std::endl;
                std::cout << "  Amount: " << transaction.getAmount() << std::endl;
                std::cout << "  Sender sent: ";
                for (const auto& sent : transaction.getSenderSent()) {
                    std::cout << sent << " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }

private:
    std::vector<Block> m_chain;
    std::vector<Transaction> m_pendingTransactions;
    int m_difficulty;
};

int main() {
    // Creare portofele
    Wallet wallet1("Alice", 1000.0);
    Wallet wallet2("Bob", 1000.0);

    // Creare blockchain
    Blockchain blockchain;

    // Adaugare tranzactii
    Transaction transaction1(wallet1, wallet2, 50.0);
    blockchain.addTransaction(transaction1);
    Transaction transaction2(wallet2, wallet1, 25.0);
    blockchain.addTransaction(transaction2);

    // Minare tranzactii
    blockchain.minePendingTransactions("Miner1");
    wallet1.receiveMoney(blockchain.getChain()[1].getTransactions());
    wallet2.receiveMoney(blockchain.getChain()[1].getTransactions());

    // Adaugare si minare alte tranzactii
    Transaction transaction3(wallet1, wallet2, 75.0);
    blockchain.addTransaction(transaction3);
    blockchain.minePendingTransactions("Miner2");
    wallet1.receiveMoney(blockchain.getChain()[2].getTransactions());

    // Adaugare si minare tranzactii pentru un alt portofel
    Wallet wallet3("Charlie", 500.0);
    Transaction transaction4(wallet2, wallet3, 100.0);
    blockchain.addTransaction(transaction4);
    Transaction transaction5(wallet3, wallet1, 50.0);
    blockchain.addTransaction(transaction5);
    blockchain.minePendingTransactions("Miner3");
    wallet2.receiveMoney(blockchain.getChain()[3].getTransactions());
    wallet3.receiveMoney(blockchain.getChain()[3].getTransactions());
    wallet1.receiveMoney(blockchain.getChain()[4].getTransactions());

    // Verificare blockchain
    std::cout << "Is blockchain valid? " << (blockchain.isValid() ? "Yes" : "No") << std::endl;

    // Afisare blockchain
    blockchain.printChain();

    // Afisare sold portofele
    std::cout << "Wallet 1 balance: " << wallet1.getBalance() << std::endl;
    std::cout << "Wallet 2 balance: " << wallet2.getBalance() << std::endl;
    std::cout << "Wallet 3 balance: " << wallet3.getBalance() << std::endl;

    return 0;
}



