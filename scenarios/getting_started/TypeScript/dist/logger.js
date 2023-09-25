"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Logger = void 0;
class Logger {
    static log(tags, message) {
        const tagsMessage = (tags && Array.isArray(tags)) ? `[${tags.join(', ')}]` : '[]';
        // eslint-disable-next-line no-console
        console.log(`[${new Date().toTimeString()}] [${tagsMessage}] ${message}`);
    }
}
exports.Logger = Logger;
//# sourceMappingURL=logger.js.map