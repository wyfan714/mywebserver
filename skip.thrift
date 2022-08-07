namespace cpp database_service

service crud{

    i32 insert_element(1: string key, 2: string value),
    string search_element(1: string key),
    i32 delete_element(1: string key),
}
