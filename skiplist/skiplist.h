

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <mutex>
#include <fstream>
#include <cmath>

# define STORE_FILE "data.txt"

std::string delimiter = ":";
std::mutex mtx;

template<typename K, typename V>

class Node {
    public:
        Node(){}
        Node(K k, V v, int );
        ~Node();
        K get_key() const;
        V get_value() const;
        void set_value(V);
        int node_level;
        // 二维指针
        Node<K, V> **forward;
    private:
        K key;
        V value;
};


template<typename K, typename V>
Node<K, V>::Node(const K k, const V v,int level){
    this->key = k;
    this->value = v;
    this->node_level = level;
    //二维指针初始化
    this->forward = new Node<K, V>*[level + 1];
    memset(this->forward, 0 ,sizeof(Node<K, V>*)*(level + 1));
};
template<typename K, typename V>
Node<K, V>::~Node(){
    delete []forward;
};
template<typename K, typename V>
K Node<K, V>::get_key()const {
    return key;
};
template<typename K, typename V>
V Node<K,V>::get_value()const{
    return value;
};
template<typename K, typename V>
class SkipList{
    public:
        SkipList(int);
        ~SkipList();
        int get_random_level();
        Node<K, V>* create_node(K, V, int);
        bool insert_element(K, V);
        std::string display_list();
        std::string search_element(K);
        bool delete_element(K);
        void dump_file();
        void load_file();
        int size();
    private:
        void get_key_value_from_string(const std::string& str, std::string *key, std::string * value);
        bool is_valid_string(const std::string& str);

        int  _max_level;
        int _skip_list_level;
        Node<K, V> *_header;

        std::ofstream _file_writer;
        std::ifstream _file_reader;

        int _element_count;

};
template<typename K, typename V>

Node<K, V>* SkipList<K, V>::create_node(const K k, const V v,int level){
    Node<K,V> *n = new Node<K, V>(k, v, level);
    return n;
}



template<typename K, typename V>
bool SkipList<K, V>::insert_element(const K key, const V value){
    //mtx.lock();
    Node<K, V> * current = this->_header;

    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K,V>*)*(_max_level + 1));
    for(int i = _skip_list_level; i >= 0 ;i--){
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];

    if(current != NULL && current->get_key() == key){
        //mtx.unlock();
        return false;
    }
    mtx.lock();
    if(current == NULL || current->get_key() != key){
        int random_level = get_random_level();
        if(random_level > _skip_list_level){
            for(int i = _skip_list_level + 1; i < random_level + 1;i ++){
                update[i] = _header;
            }
            _skip_list_level = random_level;//更新当前最大level
        }
        Node<K, V>* inserted_node = create_node(key,value, random_level);

        for(int i=0;i<=random_level;i++){
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        _element_count++;
    }
    mtx.unlock();
    return true;
}
template<typename K, typename V>

std::string SkipList<K, V>::display_list(){
    std::string s = "";
    //for(int i = 0; i <=_skip_list_level; i ++){
    //    Node<K, V> *node = this->_header->forward[i];
        //std::cout << "Level " << i << ":";
    //    while(node != NULL){
    //        s += "found Key:" + node->get_key() + ", value:" + node->get_value() +"\n";
    //        node = node->forward[i];
    //    }
        //std::cout << std::endl;
    //}
    Node<K, V> *node = this->_header->forward[0];
    while(node != NULL){
        s += "找到key:" + node->get_key() + ", value:" + node->get_value() +"\n";
        node = node->forward[0];
    }
    return s == "" ? "当前数据库没有数据！" : s;
}

template<typename K, typename V>
int SkipList<K, V>::size(){
    return _element_count;
}
template<typename K, typename V>
bool SkipList<K, V>::delete_element(K key){
    //mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));
    for(int i = _skip_list_level; i >= 0; i--){
        while(current->forward[i]!=NULL && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    mtx.lock();
    if (current != NULL && current->get_key() == key){
        for(int i = 0; i<= _skip_list_level; i++){
            if(update[i]->forward[i] != current)
                break;
            update[i]->forward[i] = current->forward[i];
        }
        while(_skip_list_level > 0 && _header->forward[_skip_list_level] == 0){
            _skip_list_level --;
        }
        _element_count --;
        mtx.unlock();
        return true;
    }
    else{
        mtx.unlock();
        return false;
    }
}
template<typename K, typename V>

std::string SkipList<K, V>::search_element(K key){
    Node<K, V> *current = _header;

    for(int i = _skip_list_level; i>=0 ;i--){
        while(current->forward[i] && current->forward[i]->get_key() < key){
            current = current->forward[i];
        }
    }
    current = current->forward[0];

    if(current and current->get_key() == key){
        std::string tmp = "找到key:" + key + ", value:" + current->get_value() +"\n";
        return tmp;
    }
    return "该key不存在!";

}
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level){
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList(){
    if(_file_writer.is_open()){
        _file_writer.close();
    }
    if(_file_reader.is_open()){
        _file_reader.close();
    }
    delete _header;
}
template<typename K, typename V>
int SkipList<K,V>::get_random_level(){
    int k = 1;
    while(rand() % 2){
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};


template<typename K, typename V>

void SkipList<K, V>::dump_file(){
    _file_writer.open(STORE_FILE);
    Node<K, V>* node = this->_header->forward[0];

    while(node != NULL){
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
        node = node->forward[0];
    }
    _file_writer.flush();
    _file_writer.close();
    return;
}

template<typename K, typename V>

void SkipList<K, V>::load_file(){

    _file_reader.open(STORE_FILE);
    std::string line;
    std::string * key = new std::string();
    std::string * value = new std::string();
    while(getline(_file_reader, line)){
        get_key_value_from_string(line,key,value);
        if(key->empty() || value->empty()){
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}
template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string &str){
    if(str.empty()){
        return false;
    }
    if(str.find(delimiter) == std::string::npos){
        return false;
    }
    return true;
}

template<typename K, typename V>

void SkipList<K, V>::get_key_value_from_string(const std:: string& str, std::string *key, std::string* value){
    if(!is_valid_string(str)){
        return;
    }
    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter)+1, str.length());
}








