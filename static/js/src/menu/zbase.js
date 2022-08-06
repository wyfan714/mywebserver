class Menu
{
    constructor(root){
        this.root = root;
        this.$menu = $(`
<div class="skip-menu">
    <div class="type-field">
        <input type="text" class="insert-line-key" placeholder="key">
        <input type="text" class="search-line" placeholder="key">
        <input type="text" class="delete-line" placeholder="key">
        <input type="text" class="insert-line-value" placeholder="value">
    </div>
    <div class="button-field">
        <button type="button" class="insert-button">插入该键值对
        </button>
        <button type="button" class="search-button">查找该值
        </button>
        <button type="button" class="delete-button">删除该键
        </button>
    </div>
    <div class="show-field">
    消息显示
    </div>
</div>
        `);
        this.root.$skip.append(this.$menu);
        this.$insert_line_key = this.$menu.find('.insert-line-key');
        this.$insert_line_value = this.$menu.find('.insert-line-value');
        this.$search_line = this.$menu.find('.search-line');
        this.$delete_line = this.$menu.find('.delete-line');
        this.$insert_button = this.$menu.find('.insert-button');
        this.$search_button = this.$menu.find('.search-button');
        this.$delete_button = this.$menu.find('.delete-button');
        this.$show_field = this.$menu.find('.show-field');
        this.start();
    }
    start()
    {
        this.add_listening_events();
    }
    add_listening_events(){
        let outer = this;
        this.$insert_button.click(function(){
            if(outer.$insert_line_key.val() === "" || outer.$insert_line_value.val() === ""){
                outer.add_message("insert key or value can't be null");
                return false;
            }
            let key = outer.$insert_line_key.val();
            let value = outer.$insert_line_value.val();
            $.ajax({
                url: "http://182.92.85.127:9999/insert",
                type: "GET",
                data:{
                    key: key,
                    value: value,
                },
                success: function(resp){
                    outer.add_message(resp.result);
                }
            });
        });
        this.$search_button.click(function(){
            let key = outer.$search_line.val();
            $.ajax({
                url: "http://182.92.85.127:9999/search",
                type: "GET",
                data:{
                    key: key,
                },
                success: function(resp){
                    outer.add_message(resp.result);
                }
            });
        });
        this.$delete_button.click(function(){
              if(outer.$delete_line.val() === ""){
                outer.add_message("delete key can't be null");
                return false;
              }
            let key = outer.$delete_line.val();
            $.ajax({
                url: "http://182.92.85.127:9999/delete",
                type: "GET",
                data:{
                    key: key,
                },
                success: function(resp){
                    outer.add_message(resp.result);
                }
            });
        });
    }
    receive_message(message){

    }
    add_message(message){
        this.$show_field.append(this.render_message(message));
        this.$show_field.scrollTop(this.$show_field[0].scrollHeight);
    }
    render_message(message){
        return $(`<div>${message}</div>`);
    }

}
