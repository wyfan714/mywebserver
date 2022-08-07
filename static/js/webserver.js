class AboutMe {
    constructor(root) {
        this.root = root;
        this.$menu = $(`
<div class="welcome-menu">
        <div align="center" class="aboutme_field" >
        我叫王一帆，就读于吉林大学计算机科学与技术学院，目前是研二，本科就读于吉林大学软件学院，家乡是广东韶关，
        对C++、Linux、网络编程、数据库等方面有一定的基础，对计算机基础也有一定的掌握，例如计算机网络和数据结构等。
        平时喜欢看动漫，玩游戏~
        </div>
        <button type="button" class="back">返回选择
        </button>
</div>
        `);
        this.$menu.hide();
        this.root.$welcome.append(this.$menu);
        this.$back = this.$menu.find('.back');
        this.start();
    }
    start() {
        this.add_listening_events();
    }
    add_listening_events() {
        let outer = this;
        this.$back.click(function () {
            outer.hide();
            outer.root.choose.show();
        });
    }
    show() {
        this.$menu.show();
    }
    hide() {
        this.$menu.hide();
    }

}

class Video {
    constructor(root) {
        this.root = root;
        this.$menu = $(`
<div class="welcome-menu">
        <div align="center" class="video_field">
         <video width="906" height="506" controls>
         <source src="../../templates/drama.mp4" type="video/mp4">
         </video>
         </div>
        <button type="button" class="back">返回选择
        </button>
</div>
        `);
        // <video width="906" height="506" controls>
        // <source src="../../templates/drama.mp4" type="video/mp4">
        // </video>
        this.$menu.hide();
        this.root.$welcome.append(this.$menu);
        this.$back = this.$menu.find('.back');
        this.start();
    }
    start() {
        this.add_listening_events();
    }
    add_listening_events() {
        let outer = this;
        this.$back.click(function () {
            outer.hide();
            outer.root.choose.show();
        });
    }
    show() {
        this.$menu.show();
    }
    hide() {
        this.$menu.hide();
    }

}









class Login {
    constructor(root) {
        this.root = root;
        this.$menu = $(`
<div class="welcome-menu">
    <div class="type-field">
        <input type="text" class="username" placeholder="用户名">
        <input type="password" class="password" placeholder="密码">
    </div>
    <div class="button-field">
        <button type="button" class="login">登录
        </button>
        <button type="button" class="register">前往注册
        </button>
    </div>
    <div class="show-field">
    历史消息
    </div>
</div>
        `);
        //this.$menu.hide();
        this.root.$welcome.append(this.$menu);
        this.$username = this.$menu.find('.username');
        this.$password = this.$menu.find('.password');
        this.$login = this.$menu.find('.login');
        this.$register = this.$menu.find('.register');
        this.$show_field = this.$menu.find('.show-field');
        this.start();
    }
    start() {
        this.add_listening_events();
    }
    add_listening_events() {
        let outer = this;
        this.$login.click(function () {
            if (outer.$username.val() === "" || outer.$password.val() === "") {
                outer.add_message("用户名或者密码不能为空");
                return false;
            }
            let username = outer.$username.val();
            let password = outer.$password.val();
            $.ajax({
                url: "http://182.92.85.127:16666/login",
                type: "POST",
                data: {
                    user: username,
                    passwd: password,
                },
                success: function (resp) {
                    if (resp === 'success') {
                        outer.hide();
                        //outer.clear();
                        outer.root.choose.show();
                    } else {
                        outer.add_message("用户名或者密码错误！");
                    }
                }
            });
        });
        this.$register.click(function () {
            outer.hide();
            outer.clear();
            outer.root.signin.show();
        });
    }
    receive_message(message) {

    }
    add_message(message) {
        this.$show_field.append(this.render_message(this.getNowTime() + message));
        this.$show_field.scrollTop(this.$show_field[0].scrollHeight);
    }
    render_message(message) {
        return $(`<div>${message}</div>`);
    }
    show() {
        this.$menu.show();
    }
    hide() {
        this.$menu.hide();
    }
    clear() {
        this.$username.val('');
        this.$password.val('');
    }
    getNowTime() {
        var date = new Date();
        //年 getFullYear()：四位数字返回年份
        var year = date.getFullYear(); //getFullYear()代替getYear()
        //月 getMonth()：0 ~ 11
        var month = date.getMonth() + 1;
        //日 getDate()：(1 ~ 31)
        var day = date.getDate();
        //时 getHours()：(0 ~ 23)
        var hour = date.getHours();
        //分 getMinutes()： (0 ~ 59)
        var minute = date.getMinutes();
        //秒 getSeconds()：(0 ~ 59)
        var second = date.getSeconds();
        var time = hour + ':' + minute + ':' + second;
        return time;
    }
}


class Signin {
    constructor(root) {
        this.root = root;
        this.$menu = $(`
<div class="welcome-menu">
    <div class="type-field">
        <input type="text" class="username" placeholder="用户名">
        <input type="password" class="password" placeholder="密码">
    </div>
    <div class="button-field">
        <button type="button" class="login">点击注册
        </button>
        <button type="button" class="register">返回登录
        </button>
    </div>
    <div class="show-field">
    历史消息
    </div>
</div>
        `);
        this.$menu.hide();
        this.root.$welcome.append(this.$menu);
        this.$username = this.$menu.find('.username');
        this.$password = this.$menu.find('.password');
        this.$login = this.$menu.find('.login');
        this.$register = this.$menu.find('.register');
        this.$show_field = this.$menu.find('.show-field');
        this.start();
    }
    start() {
        this.add_listening_events();
    }
    add_listening_events() {
        let outer = this;
        this.$login.click(function () {
            if (outer.$username.val() === "" || outer.$password.val() === "") {
                outer.add_message("用户名或者密码不能为空");
                return false;
            }
            let username = outer.$username.val();
            let password = outer.$password.val();
            $.ajax({
                url: "http://182.92.85.127:16666/register",
                type: "POST",
                data: {
                    user: username,
                    passwd: password,
                },
                success: function (resp) {
                    if (resp === 'success') {
                        outer.hide();
                        //outer.clear();
                        outer.root.login.show();
                    } else {
                        outer.add_message("用户已存在！");
                    }
                }
            });
        });
        this.$register.click(function () {
            outer.hide();
            outer.clear();
            outer.root.login.show();
        });
    }
    receive_message(message) {

    }
    add_message(message) {
        this.$show_field.append(this.render_message(this.getNowTime() + message));
        this.$show_field.scrollTop(this.$show_field[0].scrollHeight);
    }
    render_message(message) {
        return $(`<div>${message}</div>`);
    }
    show() {
        this.$menu.show();
    }
    hide() {
        this.$menu.hide();
    }
    clear() {
        this.$username.val('');
        this.$password.val('');
    }
    getNowTime() {
        var date = new Date();
        //年 getFullYear()：四位数字返回年份
        var year = date.getFullYear(); //getFullYear()代替getYear()
        //月 getMonth()：0 ~ 11
        var month = date.getMonth() + 1;
        //日 getDate()：(1 ~ 31)
        var day = date.getDate();
        //时 getHours()：(0 ~ 23)
        var hour = date.getHours();
        //分 getMinutes()： (0 ~ 59)
        var minute = date.getMinutes();
        //秒 getSeconds()：(0 ~ 59)
        var second = date.getSeconds();
        var time = hour + ':' + minute + ':' + second;
        return time;
    }
}


class Choose {
    constructor(root) {
        this.root = root;
        this.$menu = $(`
<div class="welcome-menu">
    <div class="choose-menu-field">
        <div class="choose-menu-field-item about-me">
        了解我
        </div>
        <br>
        <div class="choose-menu-field-item database">
        我的数据库项目
        </div>
        <br>
        <div class="choose-menu-field-item video">
        看个视频？
        </div>
        <br>
        <div class="choose-menu-field-item game">
        玩个游戏？
        </div>
    </div>
</div>
        `);
        this.$menu.hide();
        this.root.$welcome.append(this.$menu);
        this.$about_me = this.$menu.find('.about-me');
        this.$database = this.$menu.find('.database');
        this.$video = this.$menu.find('.video');
        this.$game = this.$menu.find('.game');
        this.start();
    }
    start() {
        this.add_listening_events();
    }
    add_listening_events() {
        let outer = this;
        this.$about_me.click(function () {
            outer.hide();
            outer.root.aboutme.show();
        });
        this.$database.click(function () {
            outer.hide();
            outer.root.database.show();
        });
        this.$video.click(function () {
            outer.hide();
            outer.root.video.show();
        });
        this.$game.click(function () {
            outer.hide();
            outer.root.game.show();
        });
    }
    show() {
        this.$menu.show();
    }
    hide() {
        this.$menu.hide();
    }

}


class Database {
    constructor(root) {
        this.root = root;
        this.$menu = $(`
<div class="welcome-menu">
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
    <button type="button" class="back">返回选择
    </button>
</div>
        `);
        this.$menu.hide();
        this.root.$welcome.append(this.$menu);
        this.$insert_line_key = this.$menu.find('.insert-line-key');
        this.$insert_line_value = this.$menu.find('.insert-line-value');
        this.$search_line = this.$menu.find('.search-line');
        this.$delete_line = this.$menu.find('.delete-line');
        this.$insert_button = this.$menu.find('.insert-button');
        this.$search_button = this.$menu.find('.search-button');
        this.$delete_button = this.$menu.find('.delete-button');
        this.$show_field = this.$menu.find('.show-field');
        this.$back = this.$menu.find('.back');
        this.start();
    }
    start() {
        this.add_listening_events();
    }
    add_listening_events() {
        let outer = this;
        this.$back.click(function () {
            outer.hide();
            outer.root.choose.show();
        });
        this.$insert_button.click(function () {
            if (outer.$insert_line_key.val() === "" || outer.$insert_line_value.val() === "") {
                outer.add_message("key和value均不能为空！");
                return false;
            }
            let key = outer.$insert_line_key.val();
            let value = outer.$insert_line_value.val();
            $.ajax({
                url: "http://182.92.85.127:16666/insert",
                type: "POST",
                data: {
                    key: key,
                    value: value,
                },
                success: function (resp) {
                    if (resp === 'success') {
                        outer.add_message("插入成功！");
                    } else {
                        outer.add_message("key已存在, 插入失败！");
                    }
                }
            });
        });
        this.$search_button.click(function () {
            let key = outer.$search_line.val();
            $.ajax({
                url: "http://182.92.85.127:16666/search",
                type: "POST",
                data: {
                    key: key,
                },
                success: function (resp) {
                    outer.add_message(resp);
                }
            });
        });
        this.$delete_button.click(function () {
            if (outer.$delete_line.val() === "") {
                outer.add_message("key不能为空！");
                return false;
            }
            let key = outer.$delete_line.val();
            $.ajax({
                url: "http://182.92.85.127:16666/delete",
                type: "POST",
                data: {
                    key: key,
                },
                success: function (resp) {
                    if (resp === 'success') {
                        outer.add_message("删除成功！");
                    } else {
                        outer.add_message("key不存在, 删除失败！");
                    }
                }
            });
        });
    }
    receive_message(message) {

    }
    add_message(message) {
        this.$show_field.append(this.render_message(message));
        this.$show_field.scrollTop(this.$show_field[0].scrollHeight);
    }
    render_message(message) {
        return $(`<div>${message}</div>`);
    }
    show() {
        this.$menu.show();
    }
    hide() {
        this.$menu.hide();
    }
}


class Game {
    constructor(root) {
        this.root = root;
        this.$menu = $(`
<div class="welcome-menu">
    <div class="choose-menu-field">
        <div class="choose-menu-field-item game-menu-field-item-single-mode">
            单人模式
        </div>
        <br>
        <div class="choose-menu-field-item game-menu-field-item-multi-mode">
        多人模式
        </div>
        <br>
        <div class="choose-menu-field-item game-menu-field-item-settings">
        退出
        </div>
    </div>
</div>
`);
        this.$menu.hide();
        this.root.$welcome.append(this.$menu);
        this.$single_mode = this.$menu.find('.game-menu-field-item-single-mode');
        this.$multi_mode = this.$menu.find('.game-menu-field-item-multi-mode');
        this.$settings = this.$menu.find('.game-menu-field-item-settings');
        this.start();
    }

    start() {
        this.add_listening_events();
    }


    add_listening_events() {
        let outer = this;
        this.$single_mode.click(function () {
            outer.hide();
            outer.root.playground.show("single mode");
        });
        this.$multi_mode.click(function () {
            outer.hide();
            outer.root.playground.show("multi mode");
        });
        this.$settings.click(function () {
            outer.hide();
            outer.root.choose.show();
        });
    }

    show() { //显示menu
        this.$menu.show();
    }
    hide() { //关闭menu
        this.$menu.hide();
    }
}


class WelCome {
    constructor(id) {
        this.id = id;
        this.$welcome = $('#' + id);
        this.login = new Login(this);
        this.signin = new Signin(this);
        this.choose = new Choose(this);
        this.aboutme = new AboutMe(this);
        this.database = new Database(this);
        this.video = new Video(this);
        this.game = new Game(this);
        this.playground = new GamePlayground(this);
    }
}

let GAME_OBJECTS = [];

class GameObject {
    constructor() {
        GAME_OBJECTS.push(this);
        this.has_called_start = false;
        this.timedelta = 0;//当前距离上一帧的时间间隔
        this.uuid = this.create_uuid();
    }

    create_uuid() {
        let res = "";
        for (let i = 0; i < 8; i++) {
            let x = parseInt(Math.floor(Math.random() * 10));
            res += x;
        }
        return res;
    }
    start() {  //只会在第一帧执行一次
    }
    update() {
    }
    late_update() {
    }
    on_destroy() {

    }
    destroy() {
        this.on_destroy();
        for (let i = 0; i < GAME_OBJECTS.length; i++) {
            if (GAME_OBJECTS[i] === this) {
                GAME_OBJECTS.splice(i, 1);
                break;
            }
        }
    }
}

let last_timestamp;

let GAME_ANIMATION = function (timestamp) {
    for (let i = 0; i < GAME_OBJECTS.length; i++) {
        let obj = GAME_OBJECTS[i];
        if (!obj.has_called_start) {
            obj.start();
            obj.has_called_start = true;
        } else {
            obj.timedelta = timestamp - last_timestamp;
            obj.update();
        }
    }
    for (let i = 0; i < GAME_OBJECTS.length; i++) {
        let obj = GAME_OBJECTS[i];
        obj.late_update();
    }
    last_timestamp = timestamp;
    requestAnimationFrame(GAME_ANIMATION);
}

requestAnimationFrame(GAME_ANIMATION);


class GameMap extends GameObject {
    constructor(playground) {
        super();
        this.playground = playground;
        this.$canvas = $(`<canvas tabindex=0></canvas>`)
        this.ctx = this.$canvas[0].getContext('2d');
        this.ctx.canvas.width = this.playground.width;
        this.ctx.canvas.height = this.playground.height;
        this.playground.$playground.append(this.$canvas);
    }
    start() {
        this.$canvas.focus();
    }
    resize() {
        this.ctx.canvas.width = this.playground.width;
        this.ctx.canvas.height = this.playground.height;
        this.ctx.fillStyle = "rgba(0,0,0,1)";
        this.ctx.fillRect(0, 0, this.ctx.canvas.width, this.ctx.canvas.height);
    }
    update() {
        this.render();
    }
    render() {
        this.ctx.fillStyle = "rgba(0,0,0,0.2)";
        this.ctx.fillRect(0, 0, this.ctx.canvas.width, this.ctx.canvas.height);
    }
}


class GamePlayground {
    constructor(root) {
        this.root = root;
        this.$playground = $(`<div class="game-playground"></div>`);
        this.root.$welcome.append(this.$playground);
        this.hide();
        this.start();
    }
    get_random_color() {
        let colors = ["blue", "red", "pink", "grey", "green"];
        return colors[Math.floor(Math.random() * 5)];
    }
    create_uuid() {
        let res = "";
        for (let i = 0; i < 8; i++) {
            let x = parseInt(Math.floor(Math.random() * 10));
            res += x;
        }
        return res;
    }
    resize() {
        this.width = this.$playground.width();
        this.height = this.$playground.height();
        // let unit = Math.min(this.width / 16, this.height / 9);
        // this.width = unit * 16;
        // this.height = unit * 9;
        this.scale = this.height;
        console.log(this.width, this.height);
        if (this.game_map) this.game_map.resize();
    }
    start() {
        // let outer = this;
        // let uuid = this.create_uuid();
    }
    show(mode) {
        let outer = this;
        this.$playground.show();
        this.width = this.$playground.width();
        this.height = this.$playground.height();
        this.game_map = new GameMap(this);
        this.mode = mode;
        this.state = "waiting"; // waiting -> fighting -> over
        this.notice_board = new NoticeBoard(this);
        this.score_board = new ScoreBoard(this);
        this.player_count = 0;
        this.resize();
        this.players = [];
        this.players.push(new Player(this, this.width / 2 / this.scale, 0.5, 0.05, "white", 0.15, "me"));
        if (mode === "single mode") {
            for (let i = 0; i < 5; i++) {
                this.players.push(new Player(this, this.width / 2 / this.scale, 0.5, 0.05, this.get_random_color(), 0.15, "robot"));
            }
        }
        else if (mode === "multi mode") {
            this.chat_field = new ChatField(this);
            this.mps = new MultiPlayerSocket(this);
            this.mps.uuid = this.players[0].uuid;
            this.mps.ws.onopen = function () {
                outer.mps.send_create_player();
            };
        }
    }
    get_random_color() {
        let colors = ["blue", "red", "pink", "grey", "green"];
        return colors[Math.floor(Math.random() * 5)];
    }
    hide() {

        while (this.players && this.players.length > 0) {
            this.players[0].destroy();
        }

        if (this.game_map) {
            this.game_map.destroy();
            this.game_map = null;
        }
        if (this.notice_board) {
            this.notice_board.destroy();
            this.notice_board = null;
        }
        if (this.score_board) {
            this.score_board.destroy();
            this.score_board = null;
        }
        this.$playground.empty();
        this.$playground.hide();
    }
}


class Player extends GameObject {
    constructor(playground, x, y, radius, color, speed, charactor) {
        super();
        this.playground = playground;
        this.ctx = this.playground.game_map.ctx;
        this.x = x;
        this.y = y;
        this.vx = 0;
        this.vy = 0;
        this.damage_x = 0;
        this.damage_y = 0;
        this.damage_speed = 0;
        this.move_length = 0;
        this.radius = radius;
        this.color = color;
        this.speed = speed;
        this.charactor = charactor;
        this.eps = 0.01;
        this.friction = 0.9;
        this.cur_skill = null;
        this.spent_time = 0;
        this.fireballs = [];

        if (this.charactor === "me") {
            this.fireball_coldtime = 3;
            this.fireball_img = new Image();
            this.fireball_img.src = "https://cdn.acwing.com/media/article/image/2021/12/02/1_9340c86053-fireball.png";
            this.blink_coldtime = 5;
            this.blink_img = new Image();
            this.blink_img.src = "https://cdn.acwing.com/media/article/image/2021/12/02/1_daccabdc53-blink.png";
        }
    }
    start() {
        this.playground.player_count++;
        this.playground.notice_board.write("已就绪：" + this.playground.player_count + "人");
        if (this.playground.player_count >= 2) {
            this.playground.state = "fighting";
            this.playground.notice_board.write("Fighting");
        }
        if (this.charactor === "me") {
            this.add_listening_events();
        } else if (this.charactor === "robot") {
            let tx = Math.random() * this.playground.width / this.playground.scale;
            let ty = Math.random() * this.playground.height / this.playground.scale;
            this.move_to(tx, ty);
        }
    }
    add_listening_events() {
        let outer = this;
        this.playground.game_map.$canvas.on("contextmenu", function () {
            return false;
        });
        this.playground.game_map.$canvas.mousedown(function (e) {
            if (outer.playground.state !== "fighting")
                return true;
            const rect = outer.ctx.canvas.getBoundingClientRect();
            if (e.which === 3) {
                let tx = (e.clientX - rect.left) / outer.playground.scale;
                let ty = (e.clientY - rect.top) / outer.playground.scale;
                outer.move_to(tx, ty);
                if (outer.playground.mode === "multi mode") {
                    outer.playground.mps.send_move_to(tx, ty);
                }
            } else if (e.which === 1) {
                let tx = (e.clientX - rect.left) / outer.playground.scale;
                let ty = (e.clientY - rect.top) / outer.playground.scale;
                if (outer.cur_skill === "fireball") {
                    if (outer.fireball_coldtime > outer.eps)
                        return false;
                    let fireball = outer.shoot_fireball(tx, ty);
                    if (outer.playground.mode === "multi mode") {
                        outer.playground.mps.send_shoot_fireball(tx, ty, fireball.uuid);
                    }
                } else if (outer.cur_skill === "blink") {
                    if (outer.blink_coldtime > outer.eps)
                        return false;
                    outer.blink(tx, ty);

                    if (outer.playground.mode === "multi mode") {
                        outer.playground.mps.send_blink(tx, ty);
                    }
                }
                outer.cur_skill = null;
            }
        });
        this.playground.game_map.$canvas.keydown(function (e) {
            if (e.which === 13) {
                 if (outer.playground.mode === "multi mode") {
                     outer.playground.chat_field.show_input();
                     return false;
                 }
            } else if (e.which === 27) {
                 if (outer.playground.mode === "multi mode") {
                     outer.playground.chat_field.hide_input();
                     return false;
                 }
            }


            if (outer.playground.state !== "fighting")
                return true;

            if (outer.fireball_coldtime > outer.eps) {
                return true;
            }
            if (e.which === 81) { //Q
                if (outer.fireball_coldtime > outer.eps) {
                    return true;
                }

                outer.cur_skill = "fireball";
                return false;
            }
            if (e.which === 70) {  //F
                if (outer.blink_coldtime > outer.eps) {
                    return true;
                }

                outer.cur_skill = "blink";
                return false;
            }
        });
    }
    shoot_fireball(tx, ty) {
        let x = this.x, y = this.y;
        let radius = 0.01;
        let angle = Math.atan2(ty - this.y, tx - this.x);
        let vx = Math.cos(angle), vy = Math.sin(angle);
        let color = "orange";
        let speed = 0.5;
        let move_length = 1;
        let fireball = new FireBall(this.playground, this, x, y, radius, vx, vy, color, speed, move_length, 0.01);
        this.fireballs.push(fireball);

        this.fireball_coldtime = 3;
        return fireball;
    }

    destroy_fireball(uuid) {
        for (let i = 0; i < this.fireballs.length; i++) {

            let fireball = this.fireballs[i];
            if (fireball.uuid === uuid) {
                fireball.destroy();
                break;
            }
        }

    }

    blink(tx, ty) {
        let d = this.get_dist(this.x, this.y, tx, ty);
        d = Math.min(d, 0.8);
        let angle = Math.atan2(ty - this.y, tx - this.x);
        this.x += d * Math.cos(angle);
        this.y += d * Math.sin(angle);

        this.blink_coldtime = 5;
        this.move_length = 0; // 闪现后不动
    }


    get_dist(x1, y1, x2, y2) {
        let dx = x1 - x2;
        let dy = y1 - y2;
        return Math.sqrt(dx * dx + dy * dy);
    }
    move_to(tx, ty) {
        this.move_length = this.get_dist(this.x, this.y, tx, ty);
        let angle = Math.atan2(ty - this.y, tx - this.x);
        this.vx = Math.cos(angle);
        this.vy = Math.sin(angle);
    }
    is_attacked(angle, damage) {
        for (let i = 0; i < 10 + Math.random() * 10; i++) {
            let x = this.x, y = this.y;
            let radius = this.radius * Math.random() * 0.1;
            let angle = Math.PI * 2 * Math.random();
            let vx = Math.cos(angle), vy = Math.sin(angle);
            let color = this.color;
            let speed = this.speed * 10;
            let move_length = this.radius * Math.random() * 5;
            new Particle(this.playground, x, y, radius, color, vx, vy, speed, move_length);
        }
        this.radius -= damage;
        if (this.radius < this.eps) {
            this.destroy();
            return false;
        }
        this.damage_x = Math.cos(angle);
        this.damage_y = Math.sin(angle);
        this.damage_speed = damage * 100;
        this.speed *= 0.8;
    }
    receive_attack(x, y, angle, damage, ball_uuid, attacker) {
        attacker.destroy_fireball(ball_uuid);
        this.x = x;
        this.y = y;
        this.is_attacked(angle, damage);
    }
    update_move() {
        if (this.charactor === "robot" && this.spent_time > 4 && Math.random() < 1 / 150.0) {
            let player = this.playground.players[Math.floor(Math.random() * this.playground.players.length)];
            let tx = player.x + player.speed * this.vx * this.timedelta / 1000 * 0.3;
            let ty = player.y + player.speed * this.vy * this.timedelta / 1000 * 0.3;
            this.shoot_fireball(tx, ty);
        }
        if (this.damage_speed > this.eps) {
            this.vx = this.vy = 0;
            this.move_length = 0;
            this.x += this.damage_x * this.damage_speed * this.timedelta / 1000;
            this.y += this.damage_y * this.damage_speed * this.timedelta / 1000;
            this.damage_speed *= this.friction;
        } else {
            if (this.move_length < this.eps) {
                this.move_length = 0;
                this.vx = this.vy = 0;
                if (this.charactor === "robot") {
                    let tx = Math.random() * this.playground.width / this.playground.scale;
                    let ty = Math.random() * this.playground.height / this.playground.scale;
                    this.move_to(tx, ty);
                }
            } else {
                let moved = Math.min(this.move_length, this.speed * this.timedelta / 1000);
                this.x += this.vx * moved;
                this.y += this.vy * moved;
                this.move_length -= moved;
            }
        }
    }
    update_coldtime() {
        this.fireball_coldtime -= this.timedelta / 1000;
        this.fireball_coldtime = Math.max(this.fireball_coldtime, 0);

        this.blink_coldtime -= this.timedelta / 1000;
        this.blink_coldtime = Math.max(this.blink_coldtime, 0);
    }
    update_win() {
        if (this.playground.state === "fighting" && this.charactor === "me" && this.playground.players.length === 1) {
            this.playground.state = "over";
            this.playground.score_board.win();
        }
    }
    update() {
        this.spent_time += this.timedelta / 1000;
        this.update_win();
        if (this.charactor === "me" && this.playground.state === "fighting") {
            this.update_coldtime();
        }
        this.update_move();
        this.render();
    }

    render_skill_coldtime() {
        let scale = this.playground.scale;
        let x = 1.5, y = 0.9, r = 0.04;
        this.ctx.save();
        this.ctx.beginPath();
        this.ctx.arc(x * scale, y * scale, r * scale, 0, Math.PI * 2, false);
        this.ctx.stroke();
        this.ctx.clip();
        this.ctx.drawImage(this.fireball_img, (x - r) * scale, (y - r) * scale, r * 2 * scale, r * 2 * scale);
        this.ctx.restore();

        if (this.fireball_coldtime > 0) {
            this.ctx.beginPath();
            this.ctx.moveTo(x * scale, y * scale);
            this.ctx.arc(x * scale, y * scale, r * scale, 0 - Math.PI / 2, Math.PI * 2 * (1 - this.fireball_coldtime / 3) - Math.PI / 2, true);
            this.ctx.lineTo(x * scale, y * scale);
            this.ctx.fillStyle = "rgba(0, 0 ,255, 0.6)";
            this.ctx.fill();
        }
        x = 1.62, y = 0.9, r = 0.04;
        this.ctx.save();
        this.ctx.beginPath();
        this.ctx.arc(x * scale, y * scale, r * scale, 0, Math.PI * 2, false);
        this.ctx.stroke();
        this.ctx.clip();
        this.ctx.drawImage(this.blink_img, (x - r) * scale, (y - r) * scale, r * 2 * scale, r * 2 * scale);
        this.ctx.restore();

        if (this.blink_coldtime > 0) {
            this.ctx.beginPath();
            this.ctx.moveTo(x * scale, y * scale);
            this.ctx.arc(x * scale, y * scale, r * scale, 0 - Math.PI / 2, Math.PI * 2 * (1 - this.blink_coldtime / 5) - Math.PI / 2, true);
            this.ctx.lineTo(x * scale, y * scale);
            this.ctx.fillStyle = "rgba(0, 0 ,255, 0.6)";
            this.ctx.fill();
        }

    }
    render() {
        let scale = this.playground.scale;
        if (this.charactor !== "robot") {
            this.ctx.beginPath();
            this.ctx.arc(this.x * scale, this.y * scale, this.radius * scale, 0, Math.PI * 2, false);
            this.ctx.fillStyle = this.color;
            this.ctx.fill();
        }
        else {
            this.ctx.beginPath();
            this.ctx.arc(this.x * scale, this.y * scale, this.radius * scale, 0, Math.PI * 2, false);
            this.ctx.fillStyle = this.color;
            this.ctx.fill();
        }
        if (this.charactor === "me" && this.playground.state === "fighting") {
            this.render_skill_coldtime();
        }
    }
    on_destroy() {
        if (this.charactor == "me") {
            if (this.playground.state === "fighting") {
                this.playground.state = "over";
                this.playground.score_board.lose();
            }
        }
        for (let i = 0; i < this.playground.players.length; i++) {
            if (this.playground.players[i] === this) {
                this.playground.players.splice(i, 1);
                break;
            }
        }
    }
}


class FireBall extends GameObject {
    constructor(playground, player, x, y, radius, vx, vy, color, speed, move_length, damage) {
        super();
        this.playground = playground;
        this.player = player;
        this.ctx = this.playground.game_map.ctx;
        this.x = x;
        this.y = y;
        this.vx = vx;
        this.vy = vy;
        this.radius = radius;
        this.color = color;
        this.speed = speed;
        this.move_length = move_length;
        this.eps = 0.01;
        this.damage = damage;
    }
    start() {
    }
    update() {
        if (this.move_length < this.eps) {
            this.destroy();
            return false;
        }
        this.update_move();
        if (this.player.character !== "enemy") {
            this.update_attack();
        }
        this.render();
    }
    update_move() {
        let moved = Math.min(this.move_length, this.speed * this.timedelta / 1000);
        this.x += this.vx * moved;
        this.y += this.vy * moved;
        this.move_length -= moved;
    }
    update_attack() {
        for (let i = 0; i < this.playground.players.length; i++) {
            let player = this.playground.players[i];
            if (this.player !== player && this.is_collision(player)) {
                this.attack(player);
                break;
            }
        }
    }

    get_dist(x1, y1, x2, y2) {
        let dx = x1 - x2;
        let dy = y1 - y2;
        return Math.sqrt(dx * dx + dy * dy);
    }
    is_collision(player) {
        let distance = this.get_dist(this.x, this.y, player.x, player.y);

        if (distance < this.radius + player.radius) {
            return true;
        }
        return false;
    }

    attack(player) {
        let angle = Math.atan2(player.y - this.y, player.x - this.x);
        player.is_attacked(angle, this.damage);

        if (this.playground.mode === "multi mode") {
            this.playground.mps.send_attack(player.uuid, player.x, player.y, angle, this.damage, this.uuid);
        }
        this.destroy();
    }
    render() {
        let scale = this.playground.scale;
        this.ctx.beginPath();
        this.ctx.arc(this.x * scale, this.y * scale, this.radius * scale, 0, Math.PI * 2, false);
        this.ctx.fillStyle = this.color;
        this.ctx.fill();
    }
    on_destroy() {
        let fireballs = this.player.fireballs;
        for (let i = 0; i < fireballs.length; i++) {
            if (fireballs[i] === this) {
                fireballs.splice(i, 1);
                break;
            }
        }

    }
}


class Particle extends GameObject {
    constructor(playground, x, y, radius, color, vx, vy, speed, move_length) {
        super();
        this.playground = playground;
        this.ctx = this.playground.game_map.ctx;
        this.x = x;
        this.y = y;
        this.radius = radius;
        this.vx = vx;
        this.vy = vy;
        this.color = color;
        this.speed = speed;
        this.friction = 0.9;
        this.eps = 0.01;
        this.move_length = move_length;
    }
    start() {
    }
    update() {
        if (this.speed < this.eps || this.move_length < this.eps) {
            this.destroy();
            return false;
        }
        let moved = Math.min(this.move_length, this.speed * this.timedelta / 1000);
        this.x += this.vx * moved;
        this.y += this.vy * moved;
        this.speed *= this.friction;
        this.move_length -= moved;
        this.render();
    }
    render() {
        let scale = this.playground.scale;
        this.ctx.beginPath();
        this.ctx.arc(this.x * scale, this.y * scale, this.radius * scale, 0, Math.PI * 2, false);
        this.ctx.fillStyle = this.color;
        this.ctx.fill();
    }
}

class ScoreBoard extends GameObject {

    constructor(playground) {
        super();
        this.playground = playground;
        this.ctx = this.playground.game_map.ctx;
        this.state = null;
        this.win_img = new Image();
        this.lose_img = new Image();
        this.win_img.src = "https://cdn.acwing.com/media/article/image/2021/12/17/1_8f58341a5e-win.png";
        this.lose_img.src = "https://cdn.acwing.com/media/article/image/2021/12/17/1_9254b5f95e-lose.png";
    }


    start() {
    }

    add_listening_events() {
        let outer = this;
        let $canvas = this.playground.game_map.$canvas;
        $canvas.on('click', function () {
            outer.playground.hide();
            //outer.playground.mps.ws.onclose = function(){};
            outer.playground.root.game.show();
        });
    }
    win() {
        this.state = "win";
        let outer = this;
        setTimeout(function () {
            outer.add_listening_events();
        }, 1000);
    }
    lose() {
        this.state = "lose";
        let outer = this;
        setTimeout(function () {
            outer.add_listening_events();
        }, 1000);

    }

    late_update() {
        this.render();
    }
    render() {

        let len = this.playground.height / 2;
        if (this.state === "win") {
            this.ctx.drawImage(this.win_img, this.playground.width / 2 - len / 2, this.playground.height / 2 - len / 2, len, len);
        } else if (this.state === "lose") {
            this.ctx.drawImage(this.lose_img, this.playground.width / 2 - len / 2, this.playground.height / 2 - len / 2, len, len);
        }
    }
}



class MultiPlayerSocket {

    constructor(playground) {
        this.playground = playground;

        this.ws = new WebSocket("ws://182.92.85.127:9002");
        this.start();
    }
    start() {
        this.receive();
    }
    receive() {
        let outer = this;
        this.ws.onmessage = function (e) {
            let data = JSON.parse(e.data);
            let uuid = data.uuid;
            if (uuid === outer.uuid) return false;
            let event = data.event;
            if (event === "create_player") {
                outer.receive_create_player(uuid);
            } else if (event === "move_to") {
                outer.receive_move_to(uuid, data.tx, data.ty);
            } else if (event === "shoot_fireball") {
                outer.receive_shoot_fireball(uuid, data.tx, data.ty, data.ball_uuid);
            } else if (event === "attack") {
                outer.receive_attack(uuid, data.attackee_uuid, data.x, data.y, data.angle, data.damage, data.ball_uuid);
            } else if (event === "blink") {
                outer.receive_blink(uuid, data.tx, data.ty);
            } else if (event === "message") {
                outer.receive_message(uuid, data.username, data.text);
            }
        };

    }
    send_create_player() {
        let outer = this;
        this.ws.send(JSON.stringify({
            'event': "create_player",
            'uuid': outer.uuid,
        }));
    }

    receive_create_player(uuid) {
        let player = new Player(
            this.playground,
            this.playground.width / 2 / this.playground.scale,
            0.5,
            0.05,
            "white",
            0.15,
            "enemy",
        );
        player.uuid = uuid;
        this.playground.players.push(player);

    }


    send_move_to(tx, ty) {
        let outer = this;
        this.ws.send(JSON.stringify({
            'event': "move_to",
            'uuid': outer.uuid,
            'tx': tx,
            'ty': ty,
        }));
    }
    get_player(uuid) {
        let players = this.playground.players;
        for (let i = 0; i < players.length; i++) {
            let player = players[i];
            if (player.uuid === uuid)
                return player;
        }
        return null;
    }
    receive_move_to(uuid, tx, ty) {
        let player = this.get_player(uuid);
        if (player) {
            player.move_to(tx, ty);
        }
    }

    send_shoot_fireball(tx, ty, ball_uuid) {
        let outer = this;
        this.ws.send(JSON.stringify({
            'event': "shoot_fireball",
            'uuid': outer.uuid,
            'tx': tx,
            'ty': ty,
            "ball_uuid": ball_uuid,
        }));
    }
    receive_shoot_fireball(uuid, tx, ty, ball_uuid) {
        let player = this.get_player(uuid);
        if (player) {
            let fireball = player.shoot_fireball(tx, ty);
            fireball.uuid = ball_uuid;
        }
    }

    send_attack(attackee_uuid, x, y, angle, damage, ball_uuid) {
        let outer = this;
        this.ws.send(JSON.stringify({
            'event': "attack",
            'uuid': outer.uuid,
            'attackee_uuid': attackee_uuid,
            'x': x,
            'y': y,
            'angle': angle,
            'damage': damage,
            'ball_uuid': ball_uuid,
        }));
    }

    receive_attack(uuid, attackee_uuid, x, y, angle, damage, ball_uuid) {
        let attacker = this.get_player(uuid);
        let attackee = this.get_player(attackee_uuid);
        if (attacker && attackee) {
            attackee.receive_attack(x, y, angle, damage, ball_uuid, attacker);
        }
    }
    send_blink(tx, ty) {
        let outer = this;
        this.ws.send(JSON.stringify({
            'event': "blink",
            'uuid': outer.uuid,
            'tx': tx,
            'ty': ty,

        }));
    }

    receive_blink(uuid, tx, ty) {
        let player = this.get_player(uuid);
        if (player) {
            player.blink(tx, ty);
        }
    }

     send_message(username, text) {
         let outer = this;
         this.ws.send(JSON.stringify({
             'event': "message",
             'uuid': outer.uuid,
             'username': username,
             'text': text,

         }));
     }
     receive_message(uuid, username, text) {
         this.playground.chat_field.add_message(username, text);
     }
}
class ChatField {
    constructor(playground) {
        this.playground = playground;

        this.$history = $(`<div class="ac-game-chat-field-history">历史记录</div>`);
        this.$input = $(`<input type="text" class="ac-game-chat-field-input">`);

        this.$history.hide();
        this.$input.hide();

        this.func_id = null;

        this.playground.$playground.append(this.$history);
        this.playground.$playground.append(this.$input);

        this.start();
    }

    start() {
        this.add_listening_events();
    }

    add_listening_events() {
        let outer = this;

        this.$input.keydown(function(e) {
            if (e.which === 27) {  // ESC
                outer.hide_input();
                return false;
            } else if (e.which === 13) {  // ENTER
                let username = outer.playground.root.login.$username.val();
                let text = outer.$input.val();
                if (text) {
                    outer.$input.val("");
                    outer.add_message(username, text);
                    outer.playground.mps.send_message(username, text);
                }
                return false;
            }
        });
    }

    render_message(message) {
        return $(`<div>${message}</div>`);
    }

    add_message(username, text) {
        this.show_history();
        let message = `[${username}]${text}`;
        this.$history.append(this.render_message(message));
        this.$history.scrollTop(this.$history[0].scrollHeight);
    }

    show_history() {
        let outer = this;
        this.$history.fadeIn();

        if (this.func_id) clearTimeout(this.func_id);

        this.func_id = setTimeout(function() {
            outer.$history.fadeOut();
            outer.func_id = null;
        }, 3000);
    }

    show_input() {
        this.show_history();

        this.$input.show();
        this.$input.focus();
    }

    hide_input() {
        this.$input.hide();
        this.playground.game_map.$canvas.focus();
    }
}
class NoticeBoard extends GameObject {
    constructor(playground) {
        super();

        this.playground = playground;
        this.ctx = this.playground.game_map.ctx;
        this.text = "已就绪：0人";
    }

    start() {
    }

    write(text) {
        this.text = text;
    }

    update() {
        this.render();
    }

    render() {
        this.ctx.font = "20px serif";
        this.ctx.fillStyle = "white";
        this.ctx.textAlign = "center";
        this.ctx.fillText(this.text, this.playground.width / 2, 20);
    }
}

