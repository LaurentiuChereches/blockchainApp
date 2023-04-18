#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <openssl/sha.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class Transaction {
public:
    Transaction(const std::string& sender, const std::string& recipient, double amount, double fee)
        : m_sender(sender), m_recipient(recipient), m_amount(amount), m_fee(fee) {}

    Transaction(const std::string& sender, const std::string& recipient, double amount, double fee, const std::vector<double>& senderSent)
        : m_sender(sender), m_recipient(recipient), m_amount(amount), m_fee(fee), m_senderSent(senderSent) {}

    std::string getSender() const { return m_sender; }
    std::string getRecipient() const { return m_recipient; }
    double getAmount() const { return m_amount; }
    double getFee() const { return m_fee; }

    std::string getDate() const { return m_date; }
    void setDate(const std::string& date) { m_date = date; }

    std::vector<std::string> getRecipientList() const { return m_recipientList; }
    void setRecipientList(const std::vector<std::string>& recipientList) { m_recipientList = recipientList; }

    std::vector<double> getSenderSent() const { return m_senderSent; }
    void setSenderSent(const std::vector<double>& sent) { m_senderSent = sent; }

    bool isValid() const {
        // Check if the transaction amount is greater than zero
        if (m_amount <= 0.0) {
            return false;
        }

        // Check if the sender has enough funds to send the transaction amount
        double totalSent = std::accumulate(m_senderSent.begin(), m_senderSent.end(), 0.0);
        if (totalSent < m_amount) {
            return false;
        }

        // Check if the recipient is not the same as the sender
        if (m_sender == m_recipient) {
            return false;
        }

        // Check if the recipient is in the recipient list, if any
        if (!m_recipientList.empty() && std::find(m_recipientList.begin(), m_recipientList.end(), m_recipient) == m_recipientList.end()) {
            return false;
        }

        // Check for suspicious patterns of transaction amounts from the sender
        int numTransactions = m_senderSent.size();
        if (numTransactions >= 3) {
            double avgSent = totalSent / numTransactions;
            double variance = 0.0;
            for (const auto& sent : m_senderSent) {
                variance += (sent - avgSent) * (sent - avgSent);
            }
            variance /= numTransactions;
            if (variance > 0.1 * avgSent) {
                return false;
            }
        }

        // Check for unusual transaction amounts compared to the sender's history
        if (numTransactions >= 5) {
            double minSent = *std::min_element(m_senderSent.begin(), m_senderSent.end());
            double maxSent = *std::max_element(m_senderSent.begin(), m_senderSent.end());
            double threshold = 0.1 * (maxSent - minSent) + minSent;
            if (m_amount > threshold) {
                return false;
            }
        }

        // All checks passed, the transaction is valid
        return true;
    }
    
    std::vector<Transaction> splitTransaction(const Transaction& transaction) {
        std::vector<Transaction> splitTransactions;
        double amountPerRecipient = transaction.getAmount() / transaction.getRecipientList().size();
        for (const auto& recipient : transaction.getRecipientList()) {
            splitTransactions.emplace_back(transaction.getSender(), recipient, amountPerRecipient, transaction.getSenderSent());
        }
        return splitTransactions;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "Transaction Details:\n";
        ss << "Sender: " << m_sender << "\n";
        ss << "Recipient: " << m_recipient << "\n";
        ss << "Amount: " << m_amount << "\n";
        ss << "Fee: " << m_fee << "\n";
        ss << "Date: " << m_date << "\n";
        ss << "Recipient List: ";
        if (m_recipientList.empty()) {
            ss << "None\n";
        }
        else {
            for (const auto& recipient : m_recipientList) {
                ss << recipient << " ";
            }
            ss << "\n";
        }
        ss << "Sender Sent: ";
        if (m_senderSent.empty()) {
            ss << "None\n";
        }
        else {
            for (const auto& sent : m_senderSent) {
                ss << sent << " ";
            }
            ss << "\n";
        }
        return ss.str();
    }

private:
    std::string m_sender;
    std::string m_recipient;
    double m_amount;
    double m_fee;
    std::string m_date = getTimeStamp();
    std::vector<std::string> m_recipientList;
    std::vector<double> m_senderSent;

    std::string getTimeStamp() const {
        std::time_t currentTime = std::time(nullptr);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&currentTime), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
};

class Wallet {
public:
    Wallet(const std::string& name, double balance = 0.0) : m_name(name), m_balance(balance) {}

    std::string getName() const { return m_name; }
    double getBalance() const { return m_balance; }
    std::vector<Transaction> getReceivedTransactions() const { return m_receivedTransactions; }
    std::map<std::string, std::vector<double>> getSenderSentMap() const { return m_senderSentMap; }

    void addFunds(double amount, const std::string& source) {
        // Add the amount to the wallet balance
        m_balance += amount;

        // Create a new Transaction object to represent the added funds
        Transaction transaction(source, m_name, amount, 0.05, {});
        
        // Add the transaction to the receivedTransactions list
        m_receivedTransactions.push_back(transaction);

        // Print transaction details
        std::cout << "Added funds transaction details: " << transaction.toString() << std::endl;

        // Check if the transaction has senderSent information and add it to the senderSentMap
        std::vector<double> senderSent = transaction.getSenderSent();
        if (!senderSent.empty()) {
            for (size_t i = 0; i < senderSent.size(); i++) {
                if (i < transaction.getRecipientList().size()) {
                    m_senderSentMap[transaction.getRecipientList()[i]] += senderSent[i];
                }
            }
        }
    }

    std::vector<Transaction> sendMoney(double amount, const std::vector<std::string>& recipients) {
        std::vector<Transaction> transactions;
        double fee = 0.05;

        // Check if the wallet has enough balance to send the amount
        if (m_balance < amount) {
            std::cerr << "Error: Wallet balance is insufficient" << std::endl;
            return transactions;
        }

        // If there is only one recipient, send the entire amount to that recipient
        if (recipients.size() == 1) {
            Transaction transaction(m_name, recipients[0], amount, fee);
            transactions.push_back(transaction);
            m_balance -= amount;

            // Print transaction details
            std::cout << "Sent transaction details: " << transaction.toString() << std::endl;
        }
        // If there are multiple recipients, split the transaction into multiple transactions
        else {
            Transaction transaction(m_name, "", amount, fee, {});
            transaction.setRecipientList(recipients);
            std::vector<Transaction> splitTransactions = transaction.splitTransaction(transaction);
            for (const auto& t : splitTransactions) {
                transactions.push_back(t);
                m_balance -= t.getAmount();

                // Print transaction details
                std::cout << "Sent transaction details: " << t.toString() << std::endl;
            }
        }

        // Update the senderSent list for each transaction
        std::vector<double> senderSent;
        for (const auto& transaction : transactions) {
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
        // Add money to wallet balance
        for (const auto& transaction : transactions) {
            if (transaction.getRecipient() == m_name) {
                // Check if the transaction has already been processed
                if (std::find(m_receivedTransactions.begin(), m_receivedTransactions.end(), transaction) != m_receivedTransactions.end()) {
                    std::cerr << "Error: Transaction has already been processed" << std::endl;
                    continue;
                }
                // Add the transaction amount to the balance
                m_balance += transaction.getAmount();
                // Add the transaction to the receivedTransactions list
                m_receivedTransactions.push_back(transaction);

                std::cout << "Received transaction details: " << std::endl;
                std::cout << transaction.toString() << std::endl;

                // Check if the transaction has senderSent information and add it to the senderSentMap
                std::vector<double> senderSent = transaction.getSenderSent();
                if (!senderSent.empty()) {
                    for (size_t i = 0; i < senderSent.size(); i++) {
                        if (i < transaction.getRecipientList().size()) {
                            m_senderSentMap[transaction.getRecipientList()[i]] += senderSent[i];
                        }
                    }
                }
            }
        }
    }

    std::vector<Transaction> getSentTransactions() const {
        std::vector<Transaction> sentTransactions;
        for (const auto& [sender, sent] : m_senderSentMap) {
            for (auto&& amount : sent) {
                sentTransactions.emplace_back(std::move(sender), m_name, std::move(amount));
            }
        }
        return sentTransactions;
    }

    std::vector<Transaction> getTransactionsInRange(const std::string& start_date, const std::string& end_date) const {
        std::vector<Transaction> transactionsInRange;
        for (const auto& transaction : m_receivedTransactions) {
            if (transaction.getDate() >= start_date && transaction.getDate() <= end_date) {
                transactionsInRange.push_back(transaction);
            }
        }
        return transactionsInRange;
    }

    std::vector<Transaction> getTransactionHistory(int limit = -1) const {
        std::vector<Transaction> transactionHistory = m_receivedTransactions;
        std::reverse(transactionHistory.begin(), transactionHistory.end());
        if (limit >= 0 && limit < static_cast<int>(transactionHistory.size())) {
            transactionHistory.resize(limit);
        }
        return transactionHistory;
    }

    int getTransactionCount() const {
        return m_receivedTransactions.size();
    }

    double getTotalAmountSent() const {
        double total = 0.0;
        for (const auto& [_, sent] : m_senderSentMap) {
            for (const auto& amount : sent) {
                total += amount;
            }
        }
        return total;
    }

    double getTotalAmountReceived() const {
        double total = 0.0;
        for (const auto& transaction : m_receivedTransactions) {
            total += transaction.getAmount();
        }
        return total;
    }

private:
    std::string m_name;
    double m_balance;
    std::vector<Transaction> m_receivedTransactions; // vector to store all received transactions
    std::map<std::string, std::vector<double>> m_senderSentMap; // map to store the amount sent by each sender
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

    std::string getLastBlockHash() const {
        return m_previousHash;
    }

    void mineBlock(int difficulty, const Wallet& minerWallet) {
        // Set up the target hash prefix to match the desired block creation rate
        std::string target(difficulty, '0');
        double targetSeconds = 600.0 / ((double) m_transactions.size() / 1000000.0);
        auto startTime = std::chrono::high_resolution_clock::now();
        std::string previousHash = getLastBlockHash();

        while (true) {
            // Check if it's time to create a new block
            auto currentTime = std::chrono::high_resolution_clock::now();
            double timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0;
            if (timeElapsed > targetSeconds) {
                break;
            }

            // Generate a random nonce and calculate the hash
            m_nonce = rand();
            m_hash = calculateHash();

            // Check if the hash matches the target prefix and the block size is within the limit
            if (m_hash.substr(0, difficulty) == target && calculateBlockSize() <= 1000) {
                std::cout << "Block mined: " << m_hash << std::endl;

                // Send the reward to the miner
                std::vector<Transaction> rewardTransaction = minerWallet.sendMoney(m_reward, minerWallet.getName());
                m_transactions.insert(m_transactions.begin(), rewardTransaction.begin(), rewardTransaction.end());

                // Clear the transactions list and update the previous hash
                m_previousHash = previousHash;
                m_transactions.clear();
                return;
            }
        }

        // If the loop exits, the target time has been exceeded and we need to adjust the difficulty
        int currentDifficulty = difficulty;
        double timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() / 1000.0;
        currentDifficulty = adjustDifficulty(timeElapsed, targetSeconds, currentDifficulty);
        std::cout << "Block mining failed, adjusting difficulty to " << currentDifficulty << std::endl;
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
    std::vector<Block> m_chain;

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

    int adjustDifficulty(double timeElapsed, int targetSeconds, int currentDifficulty) {
        int newDifficulty = currentDifficulty;
        if (timeElapsed < targetSeconds / 2) {
            newDifficulty++;
        } else if (timeElapsed > targetSeconds * 2) {
            newDifficulty--;
        }
        return newDifficulty;
    }

    size_t calculateBlockSize() const {
        size_t size = 0;
        for (const auto& transaction : m_transactions) {
            size += transaction.getSize();
        }
        return size;
    }

    double calculateTotalFee() const {
        double totalFee = 0.0;

        // Create a lambda function to calculate the fee for a single transaction
        auto calculateTransactionFee = [] (const Transaction& t) {
            return t.getFee();
        };

        // Use a range-based loop to calculate the fee for each transaction and add it to the total
        for (const auto& transaction : m_transactions) {
            totalFee += calculateTransactionFee(transaction);
        }

        // Return the total fee collected from all transactions in the block
        return totalFee;
    }
};

class Blockchain {
public:
    Blockchain() : m_difficulty(4), m_minerWallet(Wallet("Miner Wallet", 1000000.0)) {
        // Create the genesis block with an arbitrary previous hash
        std::vector<Transaction> transactions;
        transactions.emplace_back(Transaction(Wallet("Alice", 1000000.0), Wallet("Bob", 0.0), 50.0));
        m_chain.emplace_back(Block(transactions, "0"));
        m_blocksByHash[m_chain.back().getHash()] = &m_chain.back();
    }

    void addBlock(Block block) {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Mine the new block
        block.mineBlock(m_difficulty, m_minerWallet);

        // Check if a block with the same hash already exists
        auto it = m_blocksByHash.find(block.getHash());
        if (it != m_blocksByHash.end()) {
            // A block with the same hash already exists, which means a fork has occurred
            Block* existingBlock = it->second;
            if (block.getIndex() <= existingBlock->getIndex()) {
                // The existing block is already in the main chain or the new block is invalid
                return;
            }
            // The new block is part of a longer chain, so we need to switch to that chain
            switchToFork(block);
            return;
        }

        // Add the block to the chain
        m_chain.push_back(block);
        m_blocksByHash[block.getHash()] = &m_chain.back();
    }

    bool isValid() const {
        std::unique_lock<std::mutex> lock(m_mutex);

        for (size_t i = 1; i < m_chain.size(); ++i) {
            Block currentBlock = m_chain[i];
            Block previousBlock = m_chain[i-1];

            // Check if the current block's hash is valid
            if (currentBlock.getHash() != currentBlock.calculateHash()) {
                std::cerr << "Block " << i << " hash is invalid" << std::endl;
                return false;
            }

            // Check if the previous hash of the current block matches the hash of the previous block
            if (currentBlock.getLastBlockHash() != previousBlock.getHash()) {
                std::cerr << "Block " << i << " previous hash is invalid" << std::endl;
                return false;
            }
        }

        // The chain is valid if all checks pass
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
    int m_difficulty;
    Wallet m_minerWallet;
    std::vector<Block> m_chain;
    std::unordered_map<std::string, Block*> m_blocksByHash;
    mutable std::mutex m_mutex;

    void switchToFork(Block& newBlock) {
        // Find the common ancestor block of the main chain and the new chain
        Block* currentBlock = &m_chain.back();
        while (currentBlock->getIndex() > newBlock.getIndex()) {
            currentBlock = &m_chain[currentBlock->getIndex() - 1];
        }

        // Roll back the main chain to the common ancestor block
        while (m_chain.back().getIndex() > currentBlock->getIndex()) {
            m_blocksByHash.erase(m_chain.back().getHash());
            m_chain.pop_back();
        }

        // Add the blocks of the new chain to the main chain
        while (currentBlock->getIndex() < newBlock.getIndex()) {
            m_chain.push_back(*currentBlock);
            m_blocksByHash[currentBlock->getHash()] = currentBlock;
            currentBlock = currentBlock->getNextBlock();
        }
        m_chain.push_back(newBlock);
        m_blocksByHash[newBlock.getHash()] = &m_chain.back();
    }
};

int main() {
    Blockchain blockchain;

    // Create some wallets
    Wallet alice("Alice", 100.0);
    Wallet bob("Bob", 50.0);
    Wallet charlie("Charlie", 0.0);

    // Alice sends money to Bob
    std::vector<Transaction> transactions = alice.sendMoney(25.0, { bob.getName() });
    blockchain.addBlock(Block(transactions, blockchain.getLastBlockHash()));

    // Bob sends money to Charlie
    transactions = bob.sendMoney(10.0, { charlie.getName() });
    blockchain.addBlock(Block(transactions, blockchain.getLastBlockHash()));

    // Print out the blockchain
    blockchain.printChain();

    return 0;
}
