"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
class Logger {
    static log(tags, message) {
        const tagsMessage = (tags && Array.isArray(tags)) ? `[${tags.join(', ')}]` : '[]';
        // eslint-disable-next-line no-console
        console.log(`[${new Date().toTimeString()}] [${tagsMessage}] ${message}`);
    }
}
exports.default = Logger;
//# sourceMappingURL=logger.mjs.map