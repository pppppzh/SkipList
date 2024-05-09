#ifndef _SKIPLIST_H
#define _SKIPLIST_H

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <mutex>
#include <vector>
#include <memory>

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
        Node(K k, V v, int);

        typedef std::shared_ptr<Node<K, V>> ptr;
        K get_key() const { return m_key; }
        V get_value() const { return m_value; }
        void set_value(V val) { m_value = val; }
        std::vector<ptr> m_forward;
    };

    template <typename K, typename V>
    Node<K, V>::Node(K k, V v, int level)
        : m_key(k), m_value(v), m_nodeLevel(level), m_forward(level, nullptr) {}

    template <typename K, typename V>
    class SkipList
    {
    public:
        SkipList(int);
        ~SkipList();
        int get_random_level();
        Node<K, V> *create_node(K, V, int);
        int insert_element(K, V);
        void display_list();
        bool search_element(K);
        void delete_element(K);
        void dump_file();
        void load_file();
        void clear(Node<K, V> *);
        int size() { return m_elementCount; }

    private:
        void get_key_value_from_string(const std::string &, std::string &, std::string &);
        bool is_valid_string(const std::string &);

        int m_maxLevel;
        int m_skipListLevel = 0;
        Node<K, V> *m_header;
        std::ofstream m_fileWriter;
        std::ifstream m_fileReader;
        int m_elementCount = 0;

        std::mutex m_mtx;
    };

    template <typename K, typename V>
    Node<K, V> *SkipList<K, V>::create_node(const K k, V v, const int level)
    {
        Node<K, V> *n = new Node<K, V>(k, v, level);
        return n;
    }

    template <typename K, typename V>
    int SkipList<K, V>::insert_element(const K k, const V v)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        Node<K, V> *current = m_header;
        Node<K, V> *update[m_maxLevel + 1];
        memset(update, 0, sizeof(Node<K, V> *) * (m_maxLevel + 1));
        for (int i = m_skipListLevel; i >= 0; --i)
        {
            while (current->forward[i] != nullptr && current->forward[i]->get_key() < k)
            {
                current = current->forward[i];
            }
            update[i] = current;
        }
        current = current->forward[0];
        if (current != nullptr && current->get_key() == k)
        {
            std::cout << "key: " << current->get_key() << ", exists" << std::endl;
            return 1;
        }
        else
        {
            int randLevel = get_random_level();
            if (randLevel > m_skipListLevel)
            {
                for (int i = m_skipListLevel + 1; i < randLevel + 1; i++)
                {
                    update[i] = m_header;
                }
                m_skipListLevel = randLevel;
            }
            Node<K, V> *node = create_node(k, v, randLevel);
            for (int i = 0; i <= randLevel; i++)
            {
                node->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = node;
            }
            std::cout << "Successfully inserted key:" << node->get_key() << ", value:" << node->get_value() << std::endl;
            m_elementCount++;
        }
        return 0;
    }

    template <typename K, typename V>
    void SkipList<K, V>::display_list()
    {
        std::cout << "\n*****Skip List*****" << "\n";
        for (int i = 0; i < m_skipListLevel; i++)
        {
            Node<K, V> *node = m_header->forward[i];
            std::cout << "Level " << i << ": ";
            while (node != nullptr)
            {
                std::cout << node->get_key() << ":" << node->get_value() << ";";
                node = node->forward[i];
            }
            std::cout << std::endl;
        }
    }

    template <typename K, typename V>
    void SkipList<K, V>::dump_file()
    {
        std::cout << "dump_file-----------------" << std::endl;
        m_fileWriter.open(STORE_FILE);
        Node<K, V> *node = m_header->forward[0];

        while (node != nullptr)
        {
            m_fileWriter << node->get_key() << ":" << node->get_value() << "\n";
            std::cout << node->get_key() << ":" << node->get_value() << ";\n";
            node = node->forward[0];
        }

        m_fileWriter.flush();
        m_fileWriter.close();
    }

    template <typename K, typename V>
    void SkipList<K, V>::load_file()
    {
        std::cout << "load_file-----------------" << std::endl;
        m_fileReader.open(STORE_FILE);
        std::string line, key, value;
        while (getline(m_fileReader, line))
        {
            get_key_value_from_string(line, key, value);
            if (key.empty() || value.empty())
                continue;
            insert_element(key, value);
            std::cout << "key:" << key << "value:" << value << std::endl;
        }
        m_fileReader.close();
    }

    template <typename K, typename V>
    void SkipList<K, V>::get_key_value_from_string(const std::string &str, std::string &key, std::string &val)
    {

        if (!is_valid_string(str))
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
        Node<K, V> *current = m_header;
        Node<K, V> *update[m_maxLevel];
        memset(update, 0, sizeof(Node<K, V> *) * (m_maxLevel + 1));
        for (int i = m_skipListLevel; i >= 0; i--)
        {
            while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
            {
                current = current->forward[i];
            }
            update[i] = current;
        }
        current = current->forward[0];
        if (current != nullptr && current->get_key() == key)
        {
            for (int i = 0; i <= m_skipListLevel; i++)
            {
                if (update[i]->forward[i] != current)
                    break;
                update[i]->forward[i] = current->forward[i];
            }

            while (m_skipListLevel > 0 && m_header->forward[m_skipListLevel] == nullptr)
                m_skipListLevel--;

            delete current;
            m_elementCount--;
        }
    }

    template <typename K, typename V>
    bool SkipList<K, V>::search_element(K key)
    {
        Node<K, V> *current = m_header;
        for (int i = m_skipListLevel; i >= 0; i--)
        {
            while (current->forward[i] != nullptr && current->forward[i]->get_key() < key)
            {
                current = current->forward[i];
            }
        }
        current = current->forward[0];
        if (current && current->get_key() == key)
        {
            std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
            return true;
        }

        std::cout << "Not Found Key:" << key << std::endl;
        return false;
    }

    template <typename K, typename V>
    SkipList<K, V>::SkipList(int max_level)
        : m_maxLevel(max_level)
    {
        K k;
        V v;
        m_header = new Node<K, V>(k, v, max_level);
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
        if (m_header->forward[0] != nullptr)
        {
            clear(m_header->forward[0]);
        }
        delete (m_header);
    }

    template <typename K, typename V>
    void SkipList<K, V>::clear(Node<K, V> *cur)
    {
        if (cur->forward[0] != nullptr)
        {
            clear(cur->forward[0]);
        }
        delete (cur);
    }

    template <typename K, typename V>
    int SkipList<K, V>::get_random_level()
    {

        int k = 1;
        while (rand() % 2)
        {
            k++;
        }
        k = (k < m_maxLevel) ? k : m_maxLevel;
        return k;
    };
} // skiplist

#endif