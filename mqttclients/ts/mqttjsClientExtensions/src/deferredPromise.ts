export class DeferredPromise<T> {
    public then: T;
    public catch: T;
    public resolve: (value: T | PromiseLike<T>) => void;
    public reject: (value: T | PromiseLike<T>) => void;
    private promiseInternal: Promise<T>;

    constructor() {
        this.promiseInternal = new Promise<T>((resolve, reject) => {
            this.resolve = resolve;
            this.reject = reject;
        });
        this.then = this.promiseInternal.then.bind(this.promiseInternal);
        this.catch = this.promiseInternal.catch.bind(this.promiseInternal);
    }

    public get promise(): Promise<T> {
        return this.promiseInternal;
    }
}
