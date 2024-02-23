export class GeoJsonPoint {
    constructor(x: number, y: number) {
        this.coordinates[0] = x;
        this.coordinates[1] = y;
    }

    public type = 'Point';
    public coordinates: number[] = [0, 0];
}
