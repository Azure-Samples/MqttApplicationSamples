/* eslint-disable quote-props */
module.exports = {
    env: {
        es6: true,
        node: true
    },
    extends: [
        'eslint:recommended',
        'plugin:@typescript-eslint/recommended'
    ],
    parserOptions: {
        project: `${__dirname}/tsconfig.json`,
        parser: '@typescript-eslint/parser',
        sourceType: 'module'
    },
    plugins: [
        'eslint-plugin-import',
        'eslint-plugin-jsdoc',
        '@typescript-eslint'
    ],
    rules: {
        'no-shadow': 'off',
        '@typescript-eslint/no-shadow': 'error',
        'no-unused-expressions': 'off',
        '@typescript-eslint/no-unused-expressions': 'error',
        '@typescript-eslint/ban-ts-comment': 'off',
        'no-unused-vars': 'off',
        '@typescript-eslint/no-unused-vars': 'off',
        'no-unneeded-ternary': 'error',
        'array-bracket-newline': [
            'error',
            'consistent'
        ],
        'brace-style': [
            'error',
            'stroustrup'
        ],
        'comma-dangle': 'error',
        'eqeqeq': [
            'error',
            'always'
        ],
        'id-blacklist': [
            'error',
            'any',
            'Number',
            'number',
            'String',
            'string',
            'Boolean',
            'boolean',
            'Undefined',
            'undefined'
        ],
        'max-classes-per-file': [
            'off',
            1
        ],
        'max-len': [
            'error',
            {
                'code': 200
            }
        ],
        '@typescript-eslint/quotes': [
            'error',
            'single',
            {
                'allowTemplateLiterals': true
            }
        ],
        '@typescript-eslint/semi': [
            'error',
            'always'
        ],
        '@typescript-eslint/ban-types': [
            'error',
            {
                'types': {
                    'Object': {
                        'message': 'Avoid using the `Object` type. Did you mean `object`?'
                    },
                    'Function': {
                        'message': 'Avoid using the `Function` type. Prefer a specific function type, like `() => void`.'
                    },
                    'Boolean': {
                        'message': 'Avoid using the `Boolean` type. Did you mean `boolean`?'
                    },
                    'Number': {
                        'message': 'Avoid using the `Number` type. Did you mean `number`?'
                    },
                    'String': {
                        'message': 'Avoid using the `String` type. Did you mean `string`?'
                    },
                    'Symbol': {
                        'message': 'Avoid using the `Symbol` type. Did you mean `symbol`?'
                    },
                    '{}': false
                }
            }
        ],
        'indent': 'off',
        '@typescript-eslint/indent': 'error',
        '@typescript-eslint/member-delimiter-style': 'error',
        '@typescript-eslint/no-floating-promises': 'error',
        '@typescript-eslint/no-explicit-any': [
            'off',
            {
                'ignoreRestArgs': true
            }
        ],
        '@typescript-eslint/array-type': [
            'error',
            {
                'default': 'array'
            }
        ],
        '@typescript-eslint/consistent-type-assertions': 'error',
        '@typescript-eslint/consistent-type-definitions': 'error',
        '@typescript-eslint/explicit-member-accessibility': [
            'error',
            {
                'accessibility': 'explicit',
                'overrides': {
                    'constructors': 'no-public'
                }
            }
        ],
        '@typescript-eslint/member-ordering': 'off',
        '@typescript-eslint/naming-convention': [
            'error',
            {
                'selector': [
                    'enumMember',
                    'enum'
                ],
                'format': null
            }
        ],
        '@typescript-eslint/no-empty-function': 'error',
        '@typescript-eslint/no-misused-new': 'error',
        '@typescript-eslint/no-namespace': 'error',
        '@typescript-eslint/no-this-alias': 'error',
        '@typescript-eslint/no-var-requires': 'error',
        '@typescript-eslint/prefer-for-of': 'error',
        '@typescript-eslint/prefer-function-type': 'error',
        '@typescript-eslint/prefer-namespace-keyword': 'error',
        '@typescript-eslint/type-annotation-spacing': 'error',
        '@typescript-eslint/unified-signatures': 'error',
        '@typescript-eslint/explicit-module-boundary-types': [
            'error',
            {
                'allowArgumentsExplicitlyTypedAsAny': true
            }
        ],
        'object-shorthand': 'error',
        'arrow-parens': [
            'off',
            'always'
        ],
        'constructor-super': 'error',
        'curly': 'error',
        'eol-last': 'error',
        'guard-for-in': 'error',
        'import/no-extraneous-dependencies': 'error',
        'import/no-internal-modules': 'error',
        'import/order': 'off',
        'jsdoc/check-alignment': 'error',
        'jsdoc/check-indentation': 'error',
        'new-parens': 'error',
        'no-bitwise': 'error',
        'no-caller': 'error',
        'no-cond-assign': 'error',
        'no-console': 'error',
        'no-debugger': 'error',
        'no-duplicate-case': 'error',
        'no-duplicate-imports': 'error',
        'no-empty': 'error',
        'no-eval': 'error',
        'no-extra-bind': 'error',
        'no-fallthrough': 'off',
        'no-invalid-this': 'off',
        'no-irregular-whitespace': 'error',
        'no-multiple-empty-lines': 'error',
        'no-new-func': 'error',
        'no-new-wrappers': 'error',
        'no-redeclare': 'error',
        'no-return-await': 'error',
        'no-sequences': 'error',
        'no-sparse-arrays': 'error',
        'no-template-curly-in-string': 'error',
        'no-throw-literal': 'error',
        'no-trailing-spaces': 'error',
        'no-undef-init': 'error',
        'no-underscore-dangle': 'error',
        'no-unsafe-finally': 'error',
        'no-var': 'error',
        'one-var': [
            'error',
            'never'
        ],
        'prefer-const': 'error',
        'prefer-object-spread': 'error',
        'prefer-template': 'error',
        'quote-props': [
            'error',
            'as-needed'
        ],
        'radix': 'error',
        'space-before-function-paren': [
            'error',
            {
                'named': 'never',
                'asyncArrow': 'always'
            }
        ],
        'spaced-comment': [
            'error',
            'always'
        ]
    }
};
