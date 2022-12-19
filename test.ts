const message:string = "hello world!";
console.log(message);



namespace M.L {
    export namespace M.L {
        export class B<T> {
    
        }
    }
}

namespace M {

}

class A<T> extends M.L.M.L.B<T> {
    id: T extends number ? T : number;
    constructor(){
        super();
    }
}