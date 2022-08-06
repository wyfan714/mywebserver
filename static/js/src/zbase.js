class Skip{
    constructor(id){
        this.id = id;
        this.$skip = $('#' + id);
        this.menu = new Menu(this);
    }
}
