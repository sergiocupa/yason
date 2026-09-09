//  MIT License - Modified for Mandatory Attribution
//  Copyright(c) 2025 Sergio Paludo - github.com/sergiocupa
//
//  Testes de regressao do caminho JSON (parse/render).

#ifndef JSON_TESTS_H
#define JSON_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

    /* Roda a bateria de testes JSON. Devolve a quantidade de FALHAS (0 = tudo passou). */
    int json_tests_run(void);

#ifdef __cplusplus
}
#endif

#endif /* JSON_TESTS_H */
