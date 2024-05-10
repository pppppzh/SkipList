#ifndef _SKIPLIST_H
#define _SKIPLIST_H

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <vector>
#include <memory>

#define unlikely(x)     __builtin_expect(!!(x), 0)

namespace skiplist
{
#define STORE_FILE "dumpfile"
#define delimiter ":"

    template <typename K, typename V>
    class Node
    {
    private:
        K m_key;
        V m_value;
        int m_nodeLevel;

    public:
        typedef std::shared_ptr<Node<K, V>> ptr;
        Node(K k, V v, int);
        K get_key() const { return m_key; }
        V get_value() const { return m_value; }
        int getLevel() const { return m_nodeLevel; }
        void set_value(V val) { m_value = val; }
        std::vector<ptr> m_forward;
    };

    template <typename K, typename V>
    Node<K, V>::Node(K k, V v, int level)
        : m_key(k), m_value(v), m_nodeLevel(level), m_forward(level + 1, nullptr) {}

    template <typename K, typename V>
    using NodePtr = std::shared_ptr<Node<K, V>>;

    template <typename K, typename V>
    using NodeVec = std::vector<std::shared_ptr<Node<K, V>>>;

    template <typename K, typename V>
    class SkipList
    {
    public:
        SkipList(int);
        ~SkipList();

        int insert_element(K, V);
        void display_list();
        bool search_element(K);
        void delete_element(K);
        void dump_file();
        void load_file();
        int size() { return m_elementCount; }

    private:
        int get_random_level();
        NodePtr<K, V> create_node(K, V, int);
        void get_key_value_from_string(const std::string &str, std::string &key, std::string &val);
        bool is_valid_string(const std::string &);

        int m_maxLevel;
        int m_skipListLevel = 0;
        std::shared_ptr<Node<K, V>> m_header;
        std::ofstream m_fileWriter;
        std::ifstream m_fileReader;
        int m_elementCount = 0;

        std::mutex m_mtx;
    };

    template <typename K, typename V>
    SkipList<K, V>::SkipList(int max_level)
        : m_maxLevel(max_level)
    {
        m_header = std::make_shared<Node<K, V>>(K(), V(), max_level);
    }

    template <typename K, typename V>
    SkipList<K, V>::~SkipList()
    {
        if (m_fileReader.is_open())
        {
            m_fileReader.close();
        }
        if (m_fileWriter.is_open())
        {
            m_fileWriter.flush();
            m_fileWriter.close();
        }
    }

    template <typename K, typename V>
    NodePtr<K, V> SkipList<K, V>::create_node(const K k, V v, const int level)
    {
        return std::make_shared<Node<K, V>>(k, v, level);
    }

    template <typename K, typename V>
    int SkipList<K, V>::insert_element(const K k, const V v)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto current = m_header;
        auto update = NodeVec<K, V>(m_maxLevel + 1); // 记录每一层的最后一个小于k的节点(k的前一个节点)

        for (int i = m_skipListLevel; i >= 0; --i)
        {
            while (current->m_forward[i] != nullptr && current->m_forward[i]->get_key() < k)
            {
                current = current->m_forward[i];
            }
            update[i] = current;
        }
        current = current->m_forward[0];
        if (current != nullptr && current->get_key() == k)
        {
            // std::cout << "key: " << current->get_key() << ", exists" << std::endl;
            return 1;
        }
        else
        {
            int randLevel = get_random_level();
            if (randLevel > m_skipListLevel)
            {
                for (int i = m_skipListLevel + 1; i <= randLevel; i++)
                {
                    update[i] = m_header;
                }
                m_skipListLevel = randLevel;
            }
            auto node = create_node(k, v, randLevel);
            for (int i = 0; i <= randLevel; i++)
            {
                node->m_forward[i] = update[i]->m_forward[i];
                update[i]->m_forward[i] = node;
            }
            // std::cout << "Successfully inserted key:" << node->get_key() << ", value:" << node->get_value() << std::endl;
            m_elementCount++;
        }
        return 0;
    }

    template <typename K, typename V>
    void SkipList<K, V>::display_list()
    {
        std::cout << "\n*****Skip List*****" << "\n";
        for (int i = 0; i <= m_skipListLevel; i++)
        {
            auto node = m_header->m_forward[i];
            std::cout << "Level " << i << ": ";
            while (node != nullptr)
            {
                std::cout << node->get_key() << ":" << node->get_value() << ";";
                node = node->m_forward[i];
            }
            std::cout << std::endl;
        }
    }

    template <typename K, typename V>
    void SkipList<K, V>::dump_file()
    {
        // std::cout << "dump_file-----------------" << std::endl;
        m_fileWriter.open(STORE_FILE);
        auto node = m_header->m_forward[0];

        while (node != nullptr)
        {
            m_fileWriter << node->get_key() << ":" << node->get_value() << "\n";
            std::cout << node->get_key() << ":" << node->get_value() << ";\n";
            node = node->m_forward[0];
        }

        m_fileWriter.flush();
        m_fileWriter.close();
    }

    template <typename K, typename V>
    void SkipList<K, V>::load_file()
    {
        // std::cout << "load_file-----------------" << std::endl;
        m_fileReader.open(STORE_FILE);
        std::string line, key, value;
        while (getline(m_fileReader, line))
        {
            get_key_value_from_string(line, key, value);
            if (key.empty() || value.empty())
                continue;
            insert_element(key, value);
            // std::cout << "key:" << key << "value:" << value << std::endl;
        }
        m_fileReader.close();
    }

    template <typename K, typename V>
    void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string &key, std::string &val)
    {

        if (unlikely(!is_valid_string(str)))
        {
            key = val = "";
            return;
        }
        size_t index = str.find(delimiter);
        key = str.substr(0, index);
        val = str.substr(index + 1, str.length());
    }

    template <typename K, typename V>
    bool SkipList<K, V>::is_valid_string(const std::string &str)
    {

        if (str.empty())
        {
            return false;
        }
        if (str.find(delimiter) == std::string::npos)
        {
            return false;
        }
        return true;
    }

    template <typename K, typename V>
    void SkipList<K, V>::delete_element(K key)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto current = m_header;
        auto update = NodeVec<K, V>(m_maxLevel + 1);
        for (int i = m_skipListLevel; i >= 0; i--)
        {
            while (current->m_forward[i] != nullptr && current->m_forward[i]->get_key() < key)
            {
                current = current->m_forward[i];
            }
            update[i] = current;
        }
        current = current->m_forward[0];
        if (current != nullptr && current->get_key() == key)
        {
            for (int i = current->getLevel(); i >= 0; i--)
            {
                update[i]->m_forward[i] = current->m_forward[i];
            }

            while (m_skipListLevel > 0 && m_header->m_forward[m_skipListLevel] == nullptr)
                m_skipListLevel--;

            m_elementCount--;
        }
    }

    template <typename K, typename V>
    bool SkipList<K, V>::search_element(K key)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        auto current = m_header;
        for (int i = m_skipListLevel; i >= 0; i--)
        {
            while (current->m_forward[i] != nullptr && current->m_forward[i]->get_key() < key)
            {
                current = current->m_forward[i];
            }
        }
        current = current->m_forward[0];
        if (current && current->get_key() == key)
        {
            std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
            return true;
        }

        std::cout << "Not Found Key:" << key << std::endl;
        return false;
    }

    template <typename K, typename V>
    int SkipList<K, V>::get_random_level()
    {
        int k = 0;
        while (rand() % 2)
        {
            k++;
        }
        k = (k < m_maxLevel) ? k : m_maxLevel;
        return k;
    };
} // skiplist

#endif